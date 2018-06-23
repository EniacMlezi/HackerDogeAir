#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/admin/flightlist/flightlist_render.h"
#include "model/flight.h"
#include "assets.h"

int    admin_flight_list(struct http_request *);
void   admin_flight_list_error_handler(struct http_request *, int);

int 
admin_flight_list(struct http_request *req)
{
    uint32_t err;
    FlightListContext context = {
        .partial_context = {.session_id = 0}  //TODO: fill from request cookie
    };

    switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            context.flight_collection = flight_get_all_flights(&err);
            //a GET receives the home form and renders the page
            if((err = admin_flight_list_render(&context)) != (SHARED_OK))
            {
                admin_flight_list_error_handler(req, err);
                goto exit;
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string,
                strlen(context.partial_context.dst_context->string));
            goto exit;
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET exist on this page
    }  

exit:
    admin_flight_list_render_clean(&context);
    return (KORE_RESULT_OK);  
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
        shared_error_handler(req, errcode, "/admin");
    }    
}


