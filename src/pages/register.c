#include <stdbool.h>
#include <limits.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "model/user.h"
#include "assets.h"

#define REGISTER_EMAIL_VALIDATOR_INVALID    202
#define REGISTER_PASSWORD_VALIDATOR_INVALID 203

int     register_user(struct http_request *);
int     register_parseparams(struct http_request *req, user_t *user);
int     register_tryregister(user_t *);

void    register_error_handler(struct http_request *req, int errcode);

int 
register_user(struct http_request *req)
{
    int err;
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

    if((err = register_parseparams(req, &user)) != (OK)) 
    {
        register_error_handler(req, err);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = register_tryregister(&user)) != (OK))
    {
        register_error_handler(req, err);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit  
    }

    const char *success = "successfully registered";
    http_response_header(req, "content-type", "text/plain");
    http_response(req, HTTP_STATUS_OK, success, strlen(success));

    return (KORE_RESULT_OK);
}

int
register_parseparams(struct http_request *req, user_t *user)
{
    http_populate_post(req);
    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        return (REGISTER_EMAIL_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        return (REGISTER_PASSWORD_VALIDATOR_INVALID); 
    }

    return (OK);
}

int
register_tryregister(user_t *user)
{
    int returncode;

    struct kore_pgsql pgsql;
    kore_pgsql_init(&pgsql);
    
    if (!kore_pgsql_setup(&pgsql, "db", KORE_PGSQL_SYNC)) 
    {   //can't connect to db
        kore_pgsql_logerror(&pgsql);
        returncode = (SQL_DB_ERROR);
        goto out;
    }

    char outbuf[SCRYPT_MCF_LEN];
    if(!libscrypt_hash(outbuf, user->password, SCRYPT_N, SCRYPT_r, SCRYPT_p))
    {
        returncode = (HASH_ERROR);
        goto out;
    }
    user->password = outbuf;

    if (!kore_pgsql_query_params(&pgsql, "INSERT INTO \"user\" (\"email\", \"password\") VALUES ($1, $2);", 0, 2, 
        user->email, strlen(user->email), 0,
        user->password, strlen(user->password), 0))
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        returncode = (SQL_QUERY_ERROR);
        goto out;    
    }

    returncode = (OK);
    
out:
    kore_pgsql_cleanup(&pgsql);
    return returncode;
}

void
register_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch (errcode)
    {
        case (REGISTER_EMAIL_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Email format incorrect. Validator failed.");
            break;
        case (REGISTER_PASSWORD_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Password format incorrect. Validator failed.");
            break;

        default: 
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode);
    }
}