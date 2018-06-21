#include <stdbool.h>
#include <time.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/user/user_actions_render.h"
#include "model/user.h"
#include "assets.h"

int    user_actions(struct http_request *);
void   user_actions_error_handler(struct http_request *, int);

int 
user_actions(struct http_request *req)
{
    int err;
    PartialContext context = {
        .session_id = 0  //TODO: fill from request cookie
    };

    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }
    
    //a GET receives the home form and renders the page
    if((err = user_actions_render(&context)) != (SHARED_OK))
    {
        user_actions_error_handler(req, err);
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK,
        context.dst_context->string,
        strlen(context.dst_context->string));

    user_actions_render_clean(&context);
    return (KORE_RESULT_OK);    
}

void
user_actions_error_handler(struct http_request *req, int errcode)
{
    shared_error_handler(req, errcode, "/");    // redirect to user would cause recursive render 
}