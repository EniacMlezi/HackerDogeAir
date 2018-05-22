#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_http.h"
#include "model/user.h"
#include "assets.h"

int     login(struct http_request *);
bool    login_parseparams(struct http_request *, user_t *);
bool    login_trylogin(user_t *, struct http_request *);
bool    login_log_attempt(struct http_request *, struct kore_pgsql *, int, bool);

int 
login(struct http_request *req)
{
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

    if(!login_parseparams(req, &user))
    {
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if(!login_trylogin(&user, req))
    {   //when not logged in correctly, notify user.
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit  
    }

    const char *success = "success";
    http_response_header(req, "content-type", "text/plain");
    http_response(req, HTTP_STATUS_OK, success, strlen(success));

    return (KORE_RESULT_OK);
}

bool
login_parseparams(struct http_request *req, user_t *user)
{
    http_populate_post(req);
    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Invalid email. validator failed");
        return (KORE_RESULT_ERROR); 
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Invalid password. validator failed");
        return (KORE_RESULT_ERROR); 
    }

    return (KORE_RESULT_OK);
}

bool
login_trylogin(user_t *user, struct http_request *req)
{
    bool success = false; 

    struct kore_pgsql pgsql;
    kore_pgsql_init(&pgsql);
    int err;

    int db_id;
    char *db_password;

    
    if (!kore_pgsql_setup(&pgsql, "db", KORE_PGSQL_SYNC)) 
    {   //can't connect to db
        kore_pgsql_logerror(&pgsql);
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. (DEBUG:db_connect error)");
        success = false;
        goto out;
    }

    if (!kore_pgsql_query_params(&pgsql, "SELECT * FROM \"user\" WHERE \"email\" = ($1);", 0, 1, 
        user->email, strlen(user->email), 0)) 
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. (DEBUG:db_query error)");
        success = false;
        goto out;    
    }

    if(kore_pgsql_ntuples(&pgsql) != 1)
    {   //no user found with specified email
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Incorrect email or password. (DEBUG:no records)");
        success = false;
        goto out;
    }

    //TODO: if successful, use db_id for session token creation?
    db_id = kore_strtonum(kore_pgsql_getvalue(&pgsql, 0, 0), 10, 0, UINT_MAX, &err);
    if (err != KORE_RESULT_OK)
    {
        kore_log(LOG_ERR, "Could not translate db_userid str to num.");
        success = false;
        goto out;
    }
    db_password = kore_pgsql_getvalue(&pgsql, 0, 2);

    if(!libscrypt_check(db_password, user->password))
    {
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Incorrect email or password. (DEBUG:incorrect password)");
        login_log_attempt(req, &pgsql, db_id, false);
        success = false;
        goto out;
    }
    login_log_attempt(req, &pgsql, db_id, true);

    http_response(req, HTTP_STATUS_OK, asset_login_success_html, asset_len_login_success_html);
    success = true;
    
out:
    kore_pgsql_cleanup(&pgsql);
    return success;
}

bool
login_log_attempt(struct http_request *req, struct kore_pgsql *pgsql, int userid, bool success)
{
    userid = htonl(userid); //userid endianness host to network

    if (!kore_pgsql_query_params(pgsql, "INSERT INTO \"login_attempt\" "\
        "(\"userid\", \"time\", \"success\") VALUES ($1, \'now\', $2);", 0, 2, 
        (char *)&userid, sizeof(userid), 1,
        (char *)&success, sizeof(success), 1)) 
    {   //error when querying
        kore_pgsql_logerror(pgsql);
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. (DEBUG:db_query error log login attempt)");
        return false;
    }
    return true;
}