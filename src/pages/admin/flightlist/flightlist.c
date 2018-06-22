#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/admin/flightlist/flightlist_render.h"
#include "model/user.h"
#include "assets.h"

int    admin_flight_list(struct http_request *);
void   admin_flight_list_error_handler(struct http_request *, int);

int 
admin_flight_list(struct http_request *req)
{
    int err;
    FlightListContext context = {
        .partial_context = {.session_id = 0}  //TODO: fill from request cookie
    };
    SLIST_INIT(&context.flightlist);

    switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            char *departure0 = "SchipInJeHol";
            char *arrival0 = "UitjeHol";

            FlightListNode flight_node0 = {
                .flight = {
                    .identifier = 0,
                    .arrival_datetime = {0, 15, 13, 18, 12, 2018-1900, 0, 0},
                    .departure_datetime = {0, 10, 13, 18, 12, 2018-1900, 0, 0},
                    .arrival_location = arrival0,
                    .departure_location = departure0
                }
            };
            SLIST_INSERT_HEAD(&context.flightlist, &flight_node0, flights);

            char *departure1 = "SchipUitJeHol";
            char *arrival1 = "InjeHol";

            FlightListNode flight_node1 = {
                .flight = {
                    .identifier = 1,
                    .arrival_datetime = {0, 15, 13, 18, 12, 2018-1900, 0, 0},
                    .departure_datetime = {0, 10, 13, 18, 12, 2018-1900, 0, 0},
                    .arrival_location = arrival1,
                    .departure_location = departure1
                }
            };
            SLIST_INSERT_HEAD(&context.flightlist, &flight_node1, flights);
            //a GET receives the home form and renders the page
            if((err = admin_flight_list_render(&context)) != (SHARED_OK))
            {
                admin_flight_list_error_handler(req, err);
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string,
                strlen(context.partial_context.dst_context->string));

            admin_flight_list_render_clean(&context);
            return (KORE_RESULT_OK);
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET exist on this page
    }    
}

void
admin_flight_list_error_handler(struct http_request *req, int errcode)
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


