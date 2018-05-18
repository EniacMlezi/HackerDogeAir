#include <stdbool.h>
#include <limits.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "model/user.h"
#include "assets.h"

int     loginpost(struct http_request *);
bool    trylogin(char *, char *, struct http_request *);
void    error_response(struct http_request *, int, const char *);

int 
loginpost(struct http_request *req)
{
    char *email;
    char *password;

    http_populate_post(req);    //read request arguments from POST

    if(!http_argument_get_string(req, "email", &email)
        || !http_argument_get_string(req, "password", &password))
    {   //validators should avoid this case, when they don't; kill request.
        kore_log(LOG_ERR, "Could not retrieve email or password. \n"
                "email: '%s' password: '%s' ", email, password);
        return (KORE_RESULT_ERROR); //don't serve users who bypass validators
    }

    if(!trylogin(email, password, req))
    {   //when not logged in correctly, notify user.
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Incorrect email or password.");
        return (KORE_RESULT_OK);    
    }

    const char *success = "success";
    http_response_header(req, "content-type", "text/plain");
    http_response(req, HTTP_STATUS_OK, success, strlen(success));

    return (KORE_RESULT_OK);
}

bool
trylogin(char *email, char *password, struct http_request *req)
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
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. db_connect");
        success = false;
        goto out;
    }

    //TODO: kore_pgsql_query_params() not working => va_list length determination?
    if (!kore_pgsql_query_params(&pgsql, "SELECT * FROM \"user\" WHERE \"email\" = ($1);", 0, 1, 
        email, strlen(email), 0)) 
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. db_query");
        success = false;
        goto out;    
    }

    if(kore_pgsql_ntuples(&pgsql) != 1)
    {   //no user found with specified email
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Incorrect email or password.");
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
        error_response(req, HTTP_STATUS_BAD_REQUEST, "Incorrect email or password.");
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