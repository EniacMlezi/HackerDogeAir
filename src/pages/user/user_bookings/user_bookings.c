#include <stdbool.h>
#include <limits.h>
#include <mustache.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/user/user_bookings/user_bookings_render.h"
#include "model/user.h"
#include "assets.h"

int     user_bookings(struct http_request *);

void    user_bookings_error_handler(struct http_request *req, int errcode);

int
user_bookings(struct http_request *req)
{
    if(req->method != HTTP_METHOD_GET)
    {   //only server GET requests
        return (KORE_RESULT_ERROR);
    }

    int return_code = (KORE_RESULT_OK);
    int err = 0;
    UserBookingsContext context = {
        .partial_context = {.session_id = 0}
    };
    
    if((err = user_bookings_render(&context)) != (SHARED_OK))
    {
        user_bookings_error_handler(req, err);
        return_code = (KORE_RESULT_OK);
        goto exit;
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
        context.partial_context.dst_context->string, 
        strlen(context.partial_context.dst_context->string));
    
    exit:
    user_bookings_render_clean(&context);
    return return_code;
}

void
user_bookings_error_handler(struct http_request *req, int errcode)
{
    // User Bookings currently has no specific errors. Use generic handler.
    shared_error_handler(req, errcode, "/user/bookings");
}