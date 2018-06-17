#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/home/home_render.h"
#include "model/user.h"
#include "assets.h"

int    home(struct http_request *);
void   home_error_handler(struct http_request *, int, SharedContext *);

int 
home(struct http_request *req)
{
    int err;
    SharedContext context = {
        .session_id = 0  //TODO: fill from request cookie
    };
    if(req->method == HTTP_METHOD_GET)
    {   //a GET receives the home form
        if((err = home_render(&context)) != (SHARED_ERROR_OK))
        {
            home_error_handler(req, err, &context);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context.dst_context->string,
            strlen(context.dst_context->string));

        home_render_clean(&context);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only serve GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    return (KORE_RESULT_OK);
}

void
home_error_handler(struct http_request *req, int errcode, SharedContext *context)
{
    int err = 0;
    if((err = home_render(context)) != (SHARED_ERROR_OK))
    {
        home_error_handler(req, err, context);
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
        *context->dst_context->string, 
        strlen(*context->dst_context->string));

    home_render_clean(context);
}