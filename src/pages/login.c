#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "model/user.h"
#include "assets.h"

#define LOGIN_BRUTEFORCE_CHECK_INVALID   101
#define LOGIN_EMAIL_VALIDATOR_INVALID    102
#define LOGIN_PASSWORD_VALIDATOR_INVALID 103
#define LOGIN_EMAIL_INCORRECT            104 
#define LOGIN_PASSWORD_INCORRECT         105  

int    login(struct http_request *);
int    login_parseparams(struct http_request *, user_t *);
int    login_trylogin(user_t *);
int    login_check_bruteforce(struct kore_pgsql *, int);
int    login_log_attempt(struct kore_pgsql *, int, bool);
void   login_error_handler(struct http_request *, int);

int 
login(struct http_request *req)
{
    int err;
    user_t user = {0, NULL, NULL};

    if(req->method == HTTP_METHOD_GET)
    {   //a GET receives the login form
        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, asset_login_html, asset_len_login_html);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only serve GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    if((err = login_parseparams(req, &user)) != (OK))
    {
        login_error_handler(req, err);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = login_trylogin(&user)) != (OK))
    {   //when not logged in correctly, notify user.
        login_error_handler(req, err);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit  
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, asset_login_success_html, asset_len_login_success_html);

    return (KORE_RESULT_OK);
}

int
login_parseparams(struct http_request *req, user_t *user)
{
    http_populate_post(req);
    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        return (LOGIN_EMAIL_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        return (LOGIN_PASSWORD_VALIDATOR_INVALID); 
    }

    return (OK);
}

int
login_trylogin(user_t *user)
{
    struct kore_pgsql pgsql;
    kore_pgsql_init(&pgsql);
    int returncode, err;

    int db_userid;
    char *db_password;
    
    if (!kore_pgsql_setup(&pgsql, "db", KORE_PGSQL_SYNC)) 
    {   //can't connect to db
        kore_pgsql_logerror(&pgsql);
        returncode = (SQL_DB_ERROR);
        goto out;
    }

    if (!kore_pgsql_query_params(&pgsql, "SELECT userid, password FROM \"user\" WHERE \"email\" = ($1);", 0, 1, 
        user->email, strlen(user->email), 0)) 
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        returncode = (SQL_QUERY_ERROR);
        goto out;    
    }

    if(kore_pgsql_ntuples(&pgsql) != 1)
    {   //no user found with specified email
        returncode = (LOGIN_EMAIL_INCORRECT);
        goto out;
    }

    //TODO: if successful, use db_userid for session token creation?
    db_userid = kore_strtonum(kore_pgsql_getvalue(&pgsql, 0, 0), 10, 0, UINT_MAX, &err);
    if (err != KORE_RESULT_OK)
    {
        kore_log(LOG_ERR, "Could not translate db_userid str to num.");
        returncode = (SQL_RESULT_TRANSLATE_ERROR);
        goto out;
    }
    db_password = kore_pgsql_getvalue(&pgsql, 0, 1);

    if((err = login_check_bruteforce(&pgsql, db_userid)) != 0)
    {
        returncode = err;
        goto out;
    }

    if(!(err = libscrypt_check(db_password, user->password)))
    {
        if(err < 0)
            returncode = (HASH_ERROR); 
        else if(err == 0)
            returncode = (LOGIN_PASSWORD_INCORRECT);

        if((err = login_log_attempt(&pgsql, db_userid, false)) != 0)
        {
            returncode = (SQL_QUERY_ERROR);
        }
        goto out;
    }
    if((err = login_log_attempt(&pgsql, db_userid, true)) != 0)
    {
        returncode = (SQL_QUERY_ERROR);
    }
    returncode = (OK);   // all successful
    
out:
    kore_pgsql_cleanup(&pgsql);
    return returncode;
}

int
login_check_bruteforce(struct kore_pgsql *pgsql, int userid)
{
    int recent_attempt_count;
    int err;

    userid = htonl(userid); //userid endianness host to network

    if (!kore_pgsql_query_params(pgsql, "SELECT count(*), min(time) FROM \"login_attempt\" WHERE \"userid\"=($1) " \
        " AND time >= CURRENT_TIMESTAMP AT TIME ZONE \'CEST\' - INTERVAL \'5 minutes\';", 0, 1, 
        (char *)&userid, sizeof(userid), 1)) 
    {
        kore_pgsql_logerror(pgsql);
        return (SQL_QUERY_ERROR);
    }  
    recent_attempt_count = kore_strtonum(kore_pgsql_getvalue(pgsql, 0, 0), 10, 0, UINT_MAX, &err);
    if (err != KORE_RESULT_OK)
    {
        kore_log(LOG_ERR, "Could not translate count str to num");
        return (SQL_RESULT_TRANSLATE_ERROR);
    }

    if(recent_attempt_count >= 5)
    {
        return (LOGIN_BRUTEFORCE_CHECK_INVALID);
    }
    return (OK);
}

int
login_log_attempt(struct kore_pgsql *pgsql, int userid, bool success)
{
    userid = htonl(userid); //userid endianness host to network

    if (!kore_pgsql_query_params(pgsql, "INSERT INTO \"login_attempt\" "\
        "(\"userid\", \"time\", \"success\") VALUES ($1, \'now\', $2);", 0, 2, 
        (char *)&userid, sizeof(userid), 1,
        (char *)&success, sizeof(success), 1)) 
    {   //error when querying
        kore_pgsql_logerror(pgsql);
        return (SQL_QUERY_ERROR);
    }
    return (OK);
}

void
login_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch (errcode)
    {
        case (LOGIN_BRUTEFORCE_CHECK_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Your account has been locked. Please try again in several minutes.");
            break;
        case (LOGIN_EMAIL_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST,
                "Email format incorrect. Validator failed.");
            break;
        case (LOGIN_PASSWORD_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Password format incorrect. Validator failed.");
            break;
        case (LOGIN_EMAIL_INCORRECT):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST,
                "Incorrect Email or Password. (DEBUG: EMAIL_INCORRECT)");
            break;
        case (LOGIN_PASSWORD_INCORRECT):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Incorrect Email or Password. (DEBUG: PASSWORD_INCORRECT)");
            break;

        default: 
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode);
    }
}