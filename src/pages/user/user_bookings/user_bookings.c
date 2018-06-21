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
    int err = 0;
    UserBookingsContext context = {
        .partial_context = {.session_id = 0}
    };

    if(req->method != HTTP_METHOD_GET)
    {   //only server GET requests
        return (KORE_RESULT_ERROR);
    }
    //TODO: get user bookings from DataAccess layer
    SLIST_INIT(&context.userbookinglist);
    char *departure0 = "SchipInJeHol";
    char *arrival0 = "UitjeHol";

    struct tm departuretime0 = {0, 10, 13, 18, 12, 2018-1900, 0, 0};
    struct tm arrivaltime0 = {0, 15, 13, 18, 12, 2018-1900, 0, 0};

    UserBookingListNode booking_node0 = {
        .flight = {
            .arrival_datetime = arrivaltime0,
            .departure_datetime = departuretime0,
            .arrival_location = arrival0,
            .departure_location = departure0
        }
    };
    SLIST_INSERT_HEAD(&context.userbookinglist, &booking_node0, userbookings);

    char *departure1 = "SchipUitJeHol";
    char *arrival1 = "InjeHol";

    struct tm departuretime1 = {0, 10, 13, 18, 12, 2018-1900, 0, 0};
    struct tm arrivaltime1 = {0, 15, 13, 18, 12, 2018-1900, 0, 0};

    UserBookingListNode booking_node1 = {
        .flight = {
            .arrival_datetime = arrivaltime1,
            .departure_datetime = departuretime1,
            .arrival_location = arrival1,
            .departure_location = departure1
        }
    };
    
    SLIST_INSERT_HEAD(&context.userbookinglist, &booking_node1, userbookings);
    
    if((err = user_bookings_render(&context)) != (SHARED_OK))
    {
        user_bookings_error_handler(req, err);
        return (KORE_RESULT_OK);
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
        context.partial_context.dst_context->string, 
        strlen(context.partial_context.dst_context->string));

    user_bookings_render_clean(&context);
    return (KORE_RESULT_OK);
}

void
user_bookings_error_handler(struct http_request *req, int errcode)
{
    // User Bookings currently has no specific errors. Use generic handler.
    shared_error_handler(req, errcode, "/user/bookings");
}