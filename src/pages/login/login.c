#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/login/login_render.h"
#include "model/user.h"
#include "assets.h"

#define LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID   101
#define LOGIN_ERROR_EMAIL_VALIDATOR_INVALID    102
#define LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID 103
#define LOGIN_ERROR_EMAIL_INCORRECT            104 
#define LOGIN_ERROR_PASSWORD_INCORRECT         105  

int    login(struct http_request *);
int    login_parseparams(struct http_request *, User *);
int    login_trylogin(User *);
int    login_check_bruteforce(struct kore_pgsql *, int);
int    login_log_attempt(struct kore_pgsql *, int, bool);
void   login_error_handler(struct http_request *, int, LoginContext *);

int 
login(struct http_request *req)
{
    int err;
    User user = {0, NULL, NULL};
    LoginContext context = {
        .shared_context = { .session_id = 0 }, //TODO: fill from request cookie
        .user = &user
    };
    if(req->method == HTTP_METHOD_GET)
    {   //a GET receives the login form
        if((err = login_render(&context)) != (SHARED_ERROR_OK))
        {
            login_error_handler(req, err, &context);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context.shared_context.dst_context->string, 
            strlen(context.shared_context.dst_context->string));

        login_render_clean(&context);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only serve GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    if((err = login_parseparams(req, context.user)) != (SHARED_ERROR_OK))
    {
        login_error_handler(req, err, &context);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = login_trylogin(context.user)) != (SHARED_ERROR_OK))
    {   //when not logged in correctly, notify user.
        login_error_handler(req, err, &context);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit  
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, asset_login_success_html, asset_len_login_success_html);

    return (KORE_RESULT_OK);
}

int
login_parseparams(struct http_request *req, User *user)
{
    http_populate_post(req);
    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        return (LOGIN_ERROR_EMAIL_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        return (LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID); 
    }

    return (SHARED_ERROR_OK);
}

int
login_trylogin(User *user)
{
    struct kore_pgsql pgsql;
    kore_pgsql_init(&pgsql);
    int return_code;
    int err;

    int db_userid;
    char *db_password;
    
    if (!kore_pgsql_setup(&pgsql, "db", KORE_PGSQL_SYNC)) 
    {   //can't connect to db
        kore_pgsql_logerror(&pgsql);
        return_code = (SHARED_ERROR_SQL_DB_ERROR);
        goto out;
    }

    const char *query = "SELECT userid, password FROM \"user\" WHERE \"email\" = ($1);";
    if (!kore_pgsql_query_params(&pgsql, query, 0, 1, 
        user->email, strlen(user->email), 0))
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        return_code = (SHARED_ERROR_SQL_QUERY_ERROR);
        goto out;    
    }

    if(kore_pgsql_ntuples(&pgsql) != 1)
    {   //no user found with specified email
        return_code = (LOGIN_ERROR_EMAIL_INCORRECT);
        goto out;
    }

    //TODO: if successful, use db_userid for session token creation?
    db_userid = kore_strtonum(kore_pgsql_getvalue(&pgsql, 0, 0), 10, 0, UINT_MAX, &err);
    if (err != KORE_RESULT_OK)
    {
        kore_log(LOG_ERR, "Could not translate db_userid str to num.");
        return_code = (SHARED_ERROR_SQL_RESULT_TRANSLATE_ERROR);
        goto out;
    }
    db_password = kore_pgsql_getvalue(&pgsql, 0, 1);

    if((err = login_check_bruteforce(&pgsql, db_userid)) != 0)
    {
        return_code = err;
        goto out;
    }

    if(!(err = libscrypt_check(db_password, user->password)))
    {
        if(err < 0)
        {
            return_code = (SHARED_ERROR_HASH_ERROR); 
        }
        else if(err == 0)
        {
            return_code = (LOGIN_ERROR_PASSWORD_INCORRECT);
        }

        if((err = login_log_attempt(&pgsql, db_userid, false)) != 0)
        {
            return_code = (SHARED_ERROR_SQL_QUERY_ERROR);
        }
        goto out;
    }
    if((err = login_log_attempt(&pgsql, db_userid, true)) != 0)
    {
        return_code = (SHARED_ERROR_SQL_QUERY_ERROR);
    }
    return_code = (SHARED_ERROR_OK);   // all successful
    
out:
    kore_pgsql_cleanup(&pgsql);
    return return_code;
}

int
login_check_bruteforce(struct kore_pgsql *pgsql, int userid)
{
    int recent_attempt_count;
    int err;

    userid = htonl(userid); //userid endianness host to network

    const char *query = "SELECT count(*), min(time) FROM \"login_attempt\" WHERE \"userid\"=($1) " \
        " AND time >= CURRENT_TIMESTAMP AT TIME ZONE \'CEST\' - INTERVAL \'5 minutes\';";

    if (!kore_pgsql_query_params(pgsql, query, 0, 1, 
        (char *)&userid, sizeof(userid), 1)) 
    {
        kore_pgsql_logerror(pgsql);
        return (SHARED_ERROR_SQL_QUERY_ERROR);
    }  
    recent_attempt_count = kore_strtonum(kore_pgsql_getvalue(pgsql, 0, 0), 10, 0, UINT_MAX, &err);
    if (err != KORE_RESULT_OK)
    {
        kore_log(LOG_ERR, "Could not translate count str to num");
        return (SHARED_ERROR_SQL_RESULT_TRANSLATE_ERROR);
    }

    if(recent_attempt_count >= 5)
    {
        return (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID);
    }
    return (SHARED_ERROR_OK);
}

int
login_log_attempt(struct kore_pgsql *pgsql, int userid, bool success)
{
    userid = htonl(userid); //userid endianness host to network

    const char *query = "INSERT INTO \"login_attempt\" "\
        "(\"userid\", \"time\", \"success\") VALUES ($1, \'now\', $2);";

    if (!kore_pgsql_query_params(pgsql, query, 0, 2, 
        (char *)&userid, sizeof(userid), 1,
        (char *)&success, sizeof(success), 1)) 
    {   //error when querying
        kore_pgsql_logerror(pgsql);
        return (SHARED_ERROR_SQL_QUERY_ERROR);
    }
    return (SHARED_ERROR_OK);
}

void
login_error_handler(struct http_request *req, int errcode, LoginContext *context)
{
    bool handled = true;
    int err = 0;
    switch (errcode)
    {
        case (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID):
                context->error_message = 
                "Your account has been locked. Please try again in several minutes.";
            break;
        case (LOGIN_ERROR_EMAIL_VALIDATOR_INVALID):
                context->error_message = 
                "Please use a correct email address (e.g. test@example.com)";
            break;
        case (LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID):
                context->error_message = 
                "Please use a correct password (length: 8-32, may contain: a-zA-Z0-9._%+-@#$^&*() )";
            break;
        case (LOGIN_ERROR_EMAIL_INCORRECT):
        case (LOGIN_ERROR_PASSWORD_INCORRECT):
                context->error_message = 
                "Incorrect Email or Password.";
            break;

        default: 
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode);
    }
    else
    {
        if((err = login_render(context)) != (SHARED_ERROR_OK))
        {
            login_error_handler(req, err, context);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context->shared_context.dst_context->string, 
            strlen(context->shared_context.dst_context->string));

        login_render_clean(context);
    }
}