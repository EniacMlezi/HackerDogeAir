#include <stdbool.h>
#include <limits.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "model/user.h"
#include "assets.h"

int register_user(struct http_request *reg);
int register_parse_params(struct http_request *req, User *user);
int register_try_register(User *user);
void register_error_handler(struct http_request *req, int errcode);

int
register_user(struct http_request *req)
{
    int err;
    //user_t user = {0, NULL, NULL};
    User user = {0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL};

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

    if((err = register_parse_params(req, &user)) != (SHARED_ERROR_OK)) 
    {
        register_error_handler(req, err);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = register_try_register(&user)) != (SHARED_ERROR_OK))
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
register_parse_params(struct http_request *req, User *user)
{
    http_populate_post(req);
    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        return (REGISTER_ERROR_EMAIL_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        return (REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID); 
    }

    return (SHARED_ERROR_OK);
}

/* Change this to valid database interfacing code. */
int
register_try_register(User *user)
{
    int return_code;

    struct kore_pgsql pgsql;
    kore_pgsql_init(&pgsql);
    
    if (!kore_pgsql_setup(&pgsql, "DogeAir", KORE_PGSQL_SYNC)) 
    {   //can't connect to db
        kore_pgsql_logerror(&pgsql);
        return_code = (SHARED_ERROR_SQL_DB_ERROR);
        goto out;
    }

    char outbuf[SCRYPT_MCF_LEN];
    if(!libscrypt_hash(outbuf, user->password, SCRYPT_N, SCRYPT_r, SCRYPT_p))
    {
        return_code = (SHARED_ERROR_HASH_ERROR);
        goto out;
    }
    user->password = outbuf;

    const char *query = "INSERT INTO \"user\" (\"email\", \"password\") VALUES ($1, $2);";
    if (!kore_pgsql_query_params(&pgsql, query, 0, 2, 
        user->email, strlen(user->email), 0,
        user->password, strlen(user->password), 0))
    {   //error when querying
        kore_pgsql_logerror(&pgsql);
        return_code = (SHARED_ERROR_SQL_QUERY_ERROR);
        goto out;    
    }

    return_code = (SHARED_ERROR_OK);
    
out:
    kore_pgsql_cleanup(&pgsql);
    return return_code;
}

void
register_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch (errcode)
    {
        case (REGISTER_ERROR_EMAIL_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Email format incorrect. Validator failed.");
            break;
        case (REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID):
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