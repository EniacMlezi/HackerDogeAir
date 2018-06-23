#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "model/flight.h"
#include "assets.h"

int         admin_delete_flight(struct http_request *);
int         admin_delete_flight_parseparams(struct http_request *, uint32_t *);
void        admin_delete_flight_error_handler(struct http_request *, int);
int         admin_try_delete_flight(uint32_t);

int 
admin_delete_flight(struct http_request *req)
{
    int err;    

    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }

    uint32_t flightid = 0;
    if ((err = admin_delete_flight_parseparams(req, &flightid)) != (SHARED_OK))
    {
        admin_delete_flight_error_handler(req, err);
        return (KORE_RESULT_OK);
    }

    if ((err = admin_try_delete_flight(flightid)) != (SHARED_OK))
    {
        admin_delete_flight_error_handler(req, err);
        return (KORE_RESULT_OK);
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK,
        asset_flight_delete_success_html,
        asset_len_flight_delete_success_html);
    return (KORE_RESULT_OK);
}

int admin_delete_flight_parseparams(struct http_request *req, uint32_t *flightid)
{
    http_populate_get(req);
    int err = (SHARED_OK);
    if(!http_argument_get_int32(req, "id", flightid))
    {
        err = (ADMIN_DELETE_FLIGHT_ID_INVALID);
    }

    return err;
}

int
admin_try_delete_flight(uint32_t flight_identifier)
{
    int err = (SHARED_OK);
    
    if(flight_delete_by_identifier(flight_identifier) != (SHARED_OK))
    {
        err = (ADMIN_DELETE_FLIGHT_ERROR);
    }
    return err;
}

void
admin_delete_flight_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch(errcode) 
    {
        case (ADMIN_DELETE_FLIGHT_ID_INVALID):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Unknown Flight Identifier. Please try again.", "/admin/flightlist", 10);
            break;
        case (ADMIN_DELETE_FLIGHT_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Flight couldn't be deleted. Please try again.", "/admin/flightlist", 10);
            break;
        default:
            handled = false;
    }
    if (!handled) 
    {
        shared_error_handler(req, errcode, "/admin/flightlist");
    }
}