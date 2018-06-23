#include <stdbool.h>
#include <limits.h>
#include <mustache.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/user/user_detail/user_detail_render.h"
#include "model/user.h"
#include "model/session.h"
#include "assets.h"

#define USER_DETAIL_ERROR_USERNAME_VALIDATOR_INVALID 300
#define USER_DETAIL_ERROR_EMAIL_VALIDATOR_INVALID 301
#define USER_DETAIL_ERROR_FIRSTNAME_VALIDATOR_INVALID 302
#define USER_DETAIL_ERROR_LASTNAME_VALIDATOR_INVALID 303
#define USER_DETAIL_ERROR_TELNUMBER_VALIDATOR_INVALID 304

int     user_detail(struct http_request *);
int     user_detail_parseparams(struct http_request *, User *);
int     user_detail_query(int, User *);

void    user_detail_error_handler(struct http_request *req, int errcode, UserContext *);

int
user_detail(struct http_request *req)
{
    if(req->method != HTTP_METHOD_GET && req->method != HTTP_METHOD_POST)
    {
        return (KORE_RESULT_ERROR);
    }

    int return_code = (KORE_RESULT_OK);
    int err = 0;

    Session session;
    
    UserContext context = {
        .partial_context = { 
            .src_context = NULL,
            .dst_context = NULL,
            .session = &session
        }
    };
    if((err = shared_http_get_user_from_request(req, &context.user)) != (SHARED_OK))
    {
        user_detail_error_handler(req, err, &context);
        return (KORE_RESULT_OK);
    }

    switch(req->method)
    {
        case (HTTP_METHOD_GET):
        {
            //TODO: fill context.user with DataAccess Layer
            if((err = user_detail_render(&context)) != (SHARED_OK))
            {
                user_detail_error_handler(req, err, &context);
                return_code = (KORE_RESULT_OK);
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
            if((err = user_detail_parseparams(req, context.user)) != (SHARED_OK))
            {
                user_detail_error_handler(req, err, &context);
                return_code = (KORE_RESULT_OK);
            }

            if((err = user_update_details(context.user)) != (SHARED_OK))
            {
                user_detail_error_handler(req, err, &context);
                return_code = (KORE_RESULT_ERROR);
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                asset_user_detail_success_html, 
                asset_len_user_detail_success_html);

            return_code = (KORE_RESULT_OK);
            break;
        }
        default:
            return_code = (KORE_RESULT_ERROR);
            break;
    }
    user_detail_render_clean(&context);
    user_destroy(&context.user);
    return return_code;
}

int
user_detail_parseparams(struct http_request *req, User *user)
{
    http_populate_post(req);
    int err = (SHARED_OK);

    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        err = (USER_DETAIL_ERROR_EMAIL_VALIDATOR_INVALID);
    }
    if(!http_argument_get_string(req, "firstname", &(user->first_name)))
    {
        err = (USER_DETAIL_ERROR_FIRSTNAME_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "lastname", &(user->last_name)))
    {
        err = (USER_DETAIL_ERROR_LASTNAME_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "telnumber", &(user->telephone_number)))
    {
        err = (USER_DETAIL_ERROR_TELNUMBER_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "username", &(user->username)))
    {
        err = (USER_DETAIL_ERROR_USERNAME_VALIDATOR_INVALID); 
    }

    return err;
}

void
user_detail_error_handler(struct http_request *req, int errcode, UserContext *context)
{
    bool handled = true;
    int err = 0;
    switch(errcode)
    {
        case (USER_DETAIL_ERROR_USERNAME_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct username (maximum length of 255 characters, containing lower-case and upper-case letters and numbers)";
            break;
        case (USER_DETAIL_ERROR_EMAIL_VALIDATOR_INVALID):
            context->error_message = 
            "Please use a correct email address (e.g. test@example.com)";
            break;

        case (USER_DETAIL_ERROR_FIRSTNAME_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct first name (maximum length of 255 characters)";
            break;

        case (USER_DETAIL_ERROR_LASTNAME_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct last name (maximum length of 255 characters)";
            break;

        case (USER_DETAIL_ERROR_TELNUMBER_VALIDATOR_INVALID):
            context->error_message = 
            "Please enter a correct telephone number";
            break;

        default:
            handled = false;
    } 

    if(!handled)
    {
        shared_error_handler(req, errcode, "/user/detail");
    }
    else
    {
        if((err = user_detail_render(context)) != (SHARED_OK))
        {
            user_detail_error_handler(req, err, context);
        }

        http_response_header(req, "context-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context->partial_context.dst_context->string,
            strlen(context->partial_context.dst_context->string));
        
        user_detail_render_clean(context);
    }
}