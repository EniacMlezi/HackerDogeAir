#include <stdbool.h>
#include <limits.h>
#include <mustache.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/user/user_bookings/user_bookings_render.h"
#include "model/user.h"
#include "model/ticket.h"
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

    uint32_t err = 0;
    UserBookingsContext context = {
        .partial_context = {
            .src_context = NULL,
            .dst_context = NULL,
            .session_id = 0,
        }
    };

    uint32_t userid = 0;
    if((err = shared_http_get_userid_from_request(req, &userid)) != (SHARED_OK))
    {
        user_bookings_error_handler(req, err);
        goto exit;
    }

    context.ticket_collection = ticket_collection_find_by_user_identifier(userid, &err);
    if(context.ticket_collection == NULL)
    {
        switch(err)
        {
            case (DATABASE_ENGINE_ERROR_NO_RESULTS):
            case (SHARED_OK):
                break;
            default:
               user_bookings_error_handler(req, err);
               goto exit; 
        }
    }

    if(err != (SHARED_OK))
    {
        user_bookings_error_handler(req, err);
        goto exit;
    }

    if((err = user_bookings_render(&context)) != (SHARED_OK))
    {
        user_bookings_error_handler(req, err);
        goto exit;
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
        context.partial_context.dst_context->string, 
        strlen(context.partial_context.dst_context->string));
    
exit:
    user_bookings_render_clean(&context);
    ticket_collection_destroy(&context.ticket_collection);
    return (KORE_RESULT_OK);
}

void
user_bookings_error_handler(struct http_request *req, int errcode)
{
    // User Bookings currently has no specific errors. Use generic handler.
    shared_error_handler(req, errcode, "/user/bookings");
}