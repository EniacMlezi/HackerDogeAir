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

#define USER_ERROR_ID_VALIDATOR_INVALID 300
#define USER_ERROR_ID_INCORRECT         301

int     user_detail(struct http_request *);
int     user_detail_parseparams(struct http_request *, int *);
int     user_detail_query(int, User *);

void    user_detail_error_handler(struct http_request *req, int errcode, UserDetailContext *);

int
user_detail(struct http_request *req)
{
    int err = 0;
    char *email = "larsgardien@live.nl";
    User user = {25, email, NULL};
    UserDetailContext context = {
        .partial_context = {.session_id = 0},
        .user = &user
    };
    if(req->method == HTTP_METHOD_GET)
    {
        int userid;
        if((err = user_detail_parseparams(req, &userid)) != (SHARED_ERROR_OK))
        {
            user_detail_error_handler(req, err, &context);
            return (KORE_RESULT_OK);
        }

        //TODO: fill context.user with DataAccess Layer

        if((err = user_detail_render(&context)) != (SHARED_ERROR_OK))
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
    
    return (KORE_RESULT_ERROR);
}

int    
user_detail_parseparams(struct http_request *req, int *userid)
{
    http_populate_get(req);
    if(!http_argument_get_uint32(req, "id", userid))
    {
        return (USER_ERROR_ID_VALIDATOR_INVALID); 
    }
    return (SHARED_ERROR_OK);
}

void
user_detail_error_handler(struct http_request *req, int errcode, UserDetailContext *context)
{
    bool handled = true;
    int err = 0;
    switch(errcode)
    {
        case (USER_ERROR_ID_VALIDATOR_INVALID):
            context->error_message = 
            "Please supply an integer as ID";
            break;
        case (USER_ERROR_ID_INCORRECT):
            context->error_message = 
            "The supplied ID does not exist"; //TODO: should not be returned for non-admin
            break;

        default:
            handled = false;
    } 

    if(!handled)
    {
        shared_error_handler(req, errcode);
    }
    else
    {
        if((err = user_detail_render(context)) != (SHARED_ERROR_OK))
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