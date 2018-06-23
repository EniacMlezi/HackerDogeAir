#include <stdbool.h>
#include <limits.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/register/register_render.h"
#include "model/session.h"
#include "model/user.h"
#include "assets.h"


int     register_user(struct http_request *);
int     register_parse_params(struct http_request *req, User *user);
int     register_try_register(User *);
void    register_error_handler(struct http_request *req, int errcode, UserContext *context);

int
register_user(struct http_request *req)
{
    if(req->method != HTTP_METHOD_GET && req->method != HTTP_METHOD_POST)
    {
        return (KORE_RESULT_ERROR);
    }

    int err;
    int return_code;
    Session session = (Session) {
        .identifier = NULL,
        .user_identifier = 0
    };

    User user = (User) {
        .identifier = 0, 
        .role = 0,
        .email = NULL, 
        .username = NULL, 
        .first_name = NULL, 
        .last_name = NULL, 
        .telephone_number = NULL, 
        .password = NULL, 
        .doge_coin = 0, 
        .registration_datetime = (struct tm) {
            .tm_year     = 0,
            .tm_mon      = 0,
            .tm_mday     = 0,
            .tm_hour     = 0,
            .tm_min      = 0,
            .tm_sec      = 0,  
            .tm_wday     = 0,
            .tm_yday     = 0
    }};

    UserContext context = {
        .partial_context = { 
            .src_context = NULL,
            .dst_context = NULL,
            .session = &session }, //TODO: fill from request cookie
        .user = &user
    };

    if ((err = shared_http_find_session_from_request(req, &context.partial_context.session)) != (SHARED_OK))
    {
        register_error_handler(req, err, &context);
        return_code = (KORE_RESULT_OK);
    }

    switch(req->method)
    {
        case (HTTP_METHOD_GET):
        {
            if((err = register_render(&context)) != (SHARED_OK))
            {
                register_error_handler(req, err, &context);
                return_code = (KORE_RESULT_OK);
                break;
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string, 
                strlen(context.partial_context.dst_context->string));

            return_code = (KORE_RESULT_OK);
            break;
        }
        case (HTTP_METHOD_POST):
        {
            if((err = register_parse_params(req, context.user)) != (SHARED_OK)) 
            {
                register_error_handler(req, err, &context);
                return_code = (KORE_RESULT_OK);
                break;
            }

            if((err = register_try_register(context.user)) != (SHARED_OK))
            {
                register_error_handler(req, err, &context);
                return_code = (KORE_RESULT_OK);
                break;
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                asset_register_success_html,
                asset_len_register_success_html);

            return_code = (KORE_RESULT_OK);
            break;
        }
        default:
            return_code = (KORE_RESULT_ERROR);
            break;
    }

    register_render_clean(&context);
    return return_code;
}

int
register_parse_params(struct http_request *req, User *user)
{
    http_populate_post(req);
    int err = (SHARED_OK);

    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        err = (REGISTER_ERROR_EMAIL_VALIDATOR_INVALID);
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        err = (REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID);
    }
    if(!http_argument_get_string(req, "firstname", &(user->first_name)))
    {
        err = (REGISTER_ERROR_FIRSTNAME_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "lastname", &(user->last_name)))
    {
        err = (REGISTER_ERROR_LASTNAME_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "telnumber", &(user->telephone_number)))
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
    int err = 0;

    char outbuf[SCRYPT_MCF_LEN];
    if(!libscrypt_hash(outbuf, user->password, SCRYPT_N, SCRYPT_r, SCRYPT_p))
    {
        return (SHARED_ERROR_HASH_ERROR);
    }
    user->password = outbuf;

    if((err = user_insert(user)) != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void
register_error_handler(struct http_request *req, int errcode, UserContext *context)
{
    bool handled = true;
    int err = 0;
    switch (errcode)
    {
        case (REGISTER_ERROR_USERNAME_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct username (maximum length of 255 characters, containing lower-case and upper-case letters and numbers)";
            break;
        case (REGISTER_ERROR_EMAIL_VALIDATOR_INVALID):
            context->error_message = 
            "Please use a correct email address (e.g. test@example.com)";
            break;
        case (REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID):
            context->error_message = 
            "Please use a correct password (length: 8-32, may contain: a-zA-Z0-9._%+-@#$^&*() )";
            break;
        case (REGISTER_ERROR_FIRSTNAME_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct first name (maximum length of 255 characters)";
            break;
        case (REGISTER_ERROR_LASTNAME_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct last name (maximum length of 255 characters)";
            break;
        case (REGISTER_ERROR_TELNUMBER_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct telephone number";
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
        if((err = register_render(context)) != (SHARED_OK))
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