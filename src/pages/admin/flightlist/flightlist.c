#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/admin/flightlist/flightlist_render.h"
#include "model/flight.h"
#include "model/session.h"
#include "assets.h"

int    admin_flight_list(struct http_request *);
void   admin_flight_list_error_handler(struct http_request *, int);

int 
admin_flight_list(struct http_request *req)
{
    uint32_t err;
    Session session = (Session) {
        .identifier = NULL,
        .user_identifier = 0
    };
    FlightListContext context = {
        .partial_context = {.session = &session}  //TODO: fill from request cookie
    };

    if ((err = shared_http_find_session_from_request(req, &context.partial_context.session)) != (SHARED_OK))
    {
        admin_flight_list_error_handler(req, err);
    }

    switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            context.flight_collection = flight_get_all_flights(&err);
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


