#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/admin/bookinglist/bookinglist_render.h"
#include "model/user.h"
#include "model/session.h"
#include "assets.h"

int    admin_booking_list(struct http_request *);
void   admin_booking_list_error_handler(struct http_request *, int);

int 
admin_booking_list(struct http_request *req)
{
    int err;
    Session session = (Session) {
        .identifier = NULL,
        .user_identifier = 0
    };
    BookingListContext context = {
        .partial_context = { .session = &session }  //TODO: fill from request cookie
    };

    if ((err = shared_http_find_session_from_request(req, &context.partial_context.session)) != (SHARED_OK))
    {
        admin_booking_list_error_handler(req, err);
    }

    SLIST_INIT(&context.bookinglist);

    switch(req->method)
    {
        case (HTTP_METHOD_GET):
        {
            BookingListNode booking_node0 = {
                .booking = {
                    .ticket_identifier = 2,
                    .user_identifier = 1,
                    .flight_identifier = 3
                }
            };

            BookingListNode booking_node1 = {
                .booking = {
                    .ticket_identifier = 7,
                    .user_identifier = 1,
                    .flight_identifier = 4
                }
            };

            BookingListNode booking_node2 = {
                .booking = {
                    .ticket_identifier = 3,
                    .user_identifier = 89,
                    .flight_identifier = 33
                }
            };

            SLIST_INSERT_HEAD(&context.bookinglist, &booking_node0, bookings);
            SLIST_INSERT_HEAD(&context.bookinglist, &booking_node1, bookings);
            SLIST_INSERT_HEAD(&context.bookinglist, &booking_node2, bookings);

            if((err = admin_booking_list_render(&context)) != (SHARED_OK))
            {
                admin_booking_list_error_handler(req, err);
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string,
                strlen(context.partial_context.dst_context->string));

            admin_booking_list_render_clean(&context);
            return (KORE_RESULT_OK);
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET exist on this page
    }
}

void
admin_booking_list_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch(errcode) 
    {
        default:
            handled = false;
    }
    if (!handled) 
    {
        shared_error_handler(req, errcode, "");
    }
}