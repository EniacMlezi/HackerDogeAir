#include <stdbool.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "model/flight.h"
#include "assets.h"

#define FLIGHT_BOOK_RESULT_OK                   0
#define FLIGHT_BOOK_RESULT_NO_SEATS_AVAILABLE   1
#define FLIGHT_BOOK_RESULT_INSUFFICIENT_FUNDS   2

int     flight_book(struct http_request *);
int     flight_book_parseparams(struct http_request *, int *);
void    flight_book_error_handler(struct http_request *, int);

int 
flight_book(struct http_request *req)
{
    uint32_t err;

    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }
    
    int flightid = 0;
    if((err = flight_book_parseparams(req, &flightid)) != (SHARED_OK))
    {
        flight_book_error_handler(req, err);
        goto exit;
    }

    Session *session = NULL;
    if((err = shared_http_get_session_from_request(req, &session)) != (SHARED_OK))
    {
        flight_book_error_handler(req, err);
        goto exit;
    }

    uint32_t book_result = flight_book_for_user(flightid, session->user_identifier, &err);
    if(err != (SHARED_OK))
    {
        flight_book_error_handler(req, err);
        goto exit;
    }

    switch(book_result)
    {
        case (FLIGHT_BOOK_RESULT_NO_SEATS_AVAILABLE):
            flight_book_error_handler(req, (FLIGHT_BOOK_ERROR_NO_SEATS_AVAILABLE));
            goto exit;
        case (FLIGHT_BOOK_RESULT_INSUFFICIENT_FUNDS):
            flight_book_error_handler(req, (FLIGHT_BOOK_ERROR_INSUFFICIENT_FUNDS));
            goto exit;
        case (FLIGHT_BOOK_RESULT_OK):
            break;
        default:
            flight_book_error_handler(req, (FLIGHT_BOOK_ERROR_UNKNOWN_RESULT));
            goto exit;
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK,
        asset_flight_book_success_html,
        asset_len_flight_book_success_html);

exit:
    session_destroy(&session);
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
        case (FLIGHT_BOOK_ERROR_INSUFFICIENT_FUNDS):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Insufficient DogeCoins.", "/flight/search", 10);
            break;
        case (FLIGHT_BOOK_ERROR_NO_SEATS_AVAILABLE):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "No more seats are available for the selected flight.", "/flight/search", 10);
            break;
        case (FLIGHT_BOOK_ERROR_UNKNOWN_RESULT):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Internal Server error. Unknown Flight Booking result", "/flight/search", 10);
            break;
        default:
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode, "/flight/search");
    }
}