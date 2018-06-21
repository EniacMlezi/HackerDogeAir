#include <stdbool.h>
#include <limits.h>
#include <mustache.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/user/user_detail/user_detail_render.h"
#include "model/user.h"
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
    int err = 0;
    User user = {0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL};
    UserContext context = {
        .partial_context = {.session_id = 0},
        .user = &user
    };
    if(req->method == HTTP_METHOD_GET)
    {
        //TODO: fill context.user with DataAccess Layer
        if((err = user_detail_render(&context)) != (SHARED_OK))
        {
            user_detail_error_handler(req, err, &context);
            return (KORE_RESULT_OK);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context.partial_context.dst_context->string, 
            strlen(context.partial_context.dst_context->string));

        user_detail_render_clean(&context);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //don't serve any other methods
        return (KORE_RESULT_ERROR);
    }

    if((err = user_detail_parseparams(req, context.user)) != (SHARED_OK))
    {
        user_detail_error_handler(req, err, &context);
        return (KORE_RESULT_OK);
    }

    //TODO: Save edits to database with DataAccess layer
    //on error: call user detail error handler 
    kore_log(LOG_INFO, "success return");
    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, asset_user_detail_success_html, asset_len_user_detail_success_html);
    
    return (KORE_RESULT_OK);
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