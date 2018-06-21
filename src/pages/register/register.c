#include <stdbool.h>
#include <limits.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/register/register_render.h"
#include "model/user.h"
#include "assets.h"

#define REGISTER_ERROR_EMAIL_VALIDATOR_INVALID    202
#define REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID 203
#define REGISTER_ERROR_FIRSTNAME_VALIDATOR_INVALID 204
#define REGISTER_ERROR_LASTNAME_VALIDATOR_INVALID 205
#define REGISTER_ERROR_TELNUMBER_VALIDATOR_INVALID 206
#define REGISTER_ERROR_USERNAME_VALIDATOR_INVALID 207

int     register_user(struct http_request *);
int     register_parse_params(struct http_request *req, User *user);
int     register_try_register(User *);

void    register_error_handler(struct http_request *req, int errcode, UserContext *context);

int
register_user(struct http_request *req)
{
    int err;
    User user = {0, NULL, NULL, NULL, NULL, NULL, NULL};
    UserContext context = {
        .partial_context = { .session_id = 0 }, //TODO: fill from request cookie
        .user = &user
    };

    if(req->method == HTTP_METHOD_GET)
    {   //a GET request receives the register form
        if((err = register_render(&context)) != (SHARED_ERROR_OK))
        {
            register_error_handler(req, err, &context);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context.partial_context.dst_context->string, 
            strlen(context.partial_context.dst_context->string));

        register_render_clean(&context);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only serve GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    if((err = register_parse_params(req, context.user)) != (SHARED_ERROR_OK)) 
    {
        register_error_handler(req, err, &context);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = register_try_register(context.user)) != (SHARED_ERROR_OK))
    {
        register_error_handler(req, err, &context);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit  
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, asset_register_success_html, asset_len_register_success_html);

    return (KORE_RESULT_OK);
}

int
register_parse_params(struct http_request *req, User *user)
{
    http_populate_post(req);
    int err = (SHARED_ERROR_OK);
    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        err = (REGISTER_ERROR_EMAIL_VALIDATOR_INVALID);
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        err = (REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "firstname", &(user->firstname)))
    {
        err = (REGISTER_ERROR_FIRSTNAME_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "lastname", &(user->lastname)))
    {
        err = (REGISTER_ERROR_LASTNAME_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "telnumber", &(user->telnumber)))
    {
        err = (REGISTER_ERROR_TELNUMBER_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "username", &(user->username)))
    {
        err = (REGISTER_ERROR_USERNAME_VALIDATOR_INVALID); 
    }

    return err;
}

int
register_try_register(User *user)
{
    int return_code;

    struct kore_pgsql pgsql;
    kore_pgsql_init(&pgsql);
    
    if (!kore_pgsql_setup(&pgsql, "db", KORE_PGSQL_SYNC)) 
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
register_error_handler(struct http_request *req, int errcode, UserContext *context)
{
    bool handled = true;
    int err = 0;
    switch (errcode)
    {
        case (REGISTER_ERROR_EMAIL_VALIDATOR_INVALID):
            context->error_message = "Please use a correct email address (e.g. test@example.com)";
            break;
        case (REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID):
            context->error_message = "Please use a correct password (length: 8-32, may contain: a-zA-Z0-9._%+-@#$^&*() )";
            break;
        case (REGISTER_ERROR_FIRSTNAME_VALIDATOR_INVALID):
            context->error_message = "Please use a correct firstname (length: 1-255, may contain: a-zA-Z and spaces )";
            break;
        case (REGISTER_ERROR_LASTNAME_VALIDATOR_INVALID):
            context->error_message = "Please use a correct lastname (length: 1-255, may contain: a-zA-Z and spaces )";
            break;
        case (REGISTER_ERROR_TELNUMBER_VALIDATOR_INVALID):
            context->error_message = "Please use a correct telephone number";
            break;
        case (REGISTER_ERROR_USERNAME_VALIDATOR_INVALID):
            context->error_message = "Please use a correct username (length: 1-255, may contain: a-zA-Z0-9 )";
            break;

        default: 
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode, "/register");
    }
    else
    {
        if((err = register_render(context)) != (SHARED_ERROR_OK))
        {
            register_error_handler(req, err, context);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context->partial_context.dst_context->string, 
            strlen(context->partial_context.dst_context->string));

        register_render_clean(context);
    }
}