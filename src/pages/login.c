#include <stdbool.h>
#include <limits.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "model/user.h"
#include "assets.h"

struct login_request_params
{
    char *email;
    char *password;
};

int     login(struct http_request *);
bool    login_trylogin(char *, char *, struct http_request *);
bool    login_parseparams(struct http_request *req, struct login_request_params *params);
void    error_response(struct http_request *, int, const char *);

int 
login(struct http_request *req)
{
    struct login_request_params params;

    if(req->method == HTTP_METHOD_GET)
    {   //a GET receives the login form
        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, asset_login_html, asset_len_login_html);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only server GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    if(!login_parseparams(req, &params))
    {
        return (KORE_RESULT_OK);
    }

    if(!login_trylogin(params.email, params.password, req))
    {   //when not logged in correctly, notify user.
        return (KORE_RESULT_OK);    
    }

    const char *success = "success";
    http_response_header(req, "content-type", "text/plain");
    http_response(req, HTTP_STATUS_OK, success, strlen(success));

    return (KORE_RESULT_OK);
}

bool
login_parseparams(struct http_request *req, struct login_request_params *params)
{
    http_populate_post(req);
    if(!http_argument_get_string(req, "email", params->email))
    {
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Invalid email. validator failed");
        return (KORE_RESULT_ERROR); 
    }
    if(!http_argument_get_string(req, "password", params->password))
    {
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Invalid password. validator failed");
        return (KORE_RESULT_ERROR); 
    }

    return (KORE_RESULT_OK);
}

bool
login_trylogin(char *email, char *password, struct http_request *req)
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

    //TODO: kore_pgsql_query_params() not working => va_list length determination?
    if (!kore_pgsql_query_params(&pgsql, "SELECT * FROM \"user\" WHERE \"email\" = ($1);", 0, 1, 
        email, strlen(email), 0)) 
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. (DEBUG:db_query error)");
        success = false;
        goto out;    
    }

    if(kore_pgsql_ntuples(&pgsql) != 1)
    {   //no user found with specified email
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Incorrect email or password. (DEBUG:no records.)");
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

    //TODO: db password is hashed
    if(strcmp(password, db_password) != 0)
    {
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Incorrect email or password. (DEBUG:incorrect password.)");
        success = false;
        goto out;
    }

    http_response(req, HTTP_STATUS_OK, asset_login_success_html, asset_len_login_success_html);
    success = true;
    
out:
    kore_pgsql_cleanup(&pgsql);
    return success;
}

void    
error_response(struct http_request *req, int http_statuscode, const char *msg)
{
    http_response_header(req, "content-type", "text/plain");
    http_response(req, http_statuscode, (const void *)msg, strlen(msg));
}