#include <stdbool.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "model/user.h"
#include "assets.h"

#define FLIGHT_BOOK_ERROR_ID_VALIDATOR_INVALID 400

int     flight_book(struct http_request *);
int     flight_book_parseparams(struct http_request *, int *);
void    flight_book_error_handler(struct http_request *, int);

int 
flight_book(struct http_request *req)
{
    int err;

    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }
    
    int flightid = 0;
    if((err = flight_book_parseparams(req, &flightid)) != (SHARED_OK))
    {
        flight_book_error_handler(req, err);
        return (KORE_RESULT_OK);
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK,
        asset_flight_book_success_html,
        asset_len_flight_book_success_html);
    return (KORE_RESULT_OK);
}

int
flight_book_parseparams(struct http_request *req, int *flightid)
{
    http_populate_get(req);
    int err = (SHARED_OK);
    if(!http_argument_get_int32(req, "id", flightid))
    {
        err = (FLIGHT_BOOK_ERROR_ID_VALIDATOR_INVALID);
    }

    return err;
}

void
flight_book_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch(errcode)
    {
        case (FLIGHT_BOOK_ERROR_ID_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Unknown Flight Identifier. Please try again.", "/flight/search", 10);
            break;

        default:
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode, "/flight/search");
    }
}