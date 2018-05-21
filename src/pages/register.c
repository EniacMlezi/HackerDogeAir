#include <stdbool.h>
#include <limits.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_http.h"
#include "model/user.h"
#include "assets.h"

int     register_user(struct http_request *);
bool    register_parseparams(struct http_request *req, user_t *user);
bool    register_tryregister(user_t *, struct http_request *);
void    error_response(struct http_request *, int, const char *);

int 
register_user(struct http_request *req)
{
    user_t user = {0, NULL, NULL};

    if(req->method == HTTP_METHOD_GET)
    {   //a GET request receives the register form
        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, asset_register_html, asset_len_register_html);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only serve GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    if(!register_parseparams(req, &user))
    {
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if(!register_tryregister(&user, req))
    {   //when not logged in correctly, notify user
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit  
    }

    const char *success = "successfully registered";
    http_response_header(req, "content-type", "text/plain");
    http_response(req, HTTP_STATUS_OK, success, strlen(success));

    return (KORE_RESULT_OK);
}

bool
register_parseparams(struct http_request *req, user_t *user)
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
register_tryregister(user_t *user, struct http_request *req)
{
    bool success = false; 

    struct kore_pgsql pgsql;
    kore_pgsql_init(&pgsql);
    
    if (!kore_pgsql_setup(&pgsql, "db", KORE_PGSQL_SYNC)) 
    {   //can't connect to db
        kore_pgsql_logerror(&pgsql);
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. (DEBUG:db_connect error)");
        success = false;
        goto out;
    }

    char outbuf[SCRYPT_MCF_LEN];
    if(!libscrypt_hash(outbuf, user->password, SCRYPT_N, SCRYPT_r, SCRYPT_p))
    {
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. (DEBUG:hash error)");
        success = false;
        goto out;
    }
    user->password = outbuf;

    if (!kore_pgsql_query_params(&pgsql, "INSERT INTO \"user\" (\"email\", \"password\") VALUES ($1, $2);", 0, 2, 
        user->email, strlen(user->email), 0,
        user->password, strlen(user->password), 0))
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. (DEBUG:db_query error)");
        success = false;
        goto out;    
    }

    http_response(req, HTTP_STATUS_OK, asset_register_success_html, asset_len_register_success_html);
    success = true;
    
out:
    kore_pgsql_cleanup(&pgsql);
    return success;
}