#include <stdbool.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "model/user.h"
#include "model/ticket.h"
#include "model/flight.h"
#include "assets.h"

int     flight_book(struct http_request *);
int     flight_book_parseparams(struct http_request *, int *);
void    flight_book_error_handler(struct http_request *, int);

int 
flight_book(struct http_request *req)
{
    int return_code;
    int err;

    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }
    
    int flightid = 0;
    if((err = flight_book_parseparams(req, &flightid)) != (SHARED_OK))
    {
        flight_book_error_handler(req, err);
        return_code = (KORE_RESULT_OK);
        goto exit;
    }

    User *user = NULL;
    if((err = shared_http_get_user_from_request(req, &user)) != (SHARED_OK))
    {
        flight_book_error_handler(req, err);
        return_code = (KORE_RESULT_OK);
        goto exit;
    }

    Flight *flight = NULL;
    if((err = flight_find_by_identifier(flightid, uint32_t *error))

    //check funds
    kore_log(LOG_DEBUG, "coins: %d", user->doge_coin);
    if(user->doge_coin < 250)
    {
        flight_book_error_handler(req, (FLIGHT_BOOK_ERROR_INSUFFICIENT_FUNDS));
        return_code = (KORE_RESULT_OK);
        goto exit;
    }

    user->doge_coin -= 250;
    kore_log(LOG_DEBUG, "coins decremented");
    if((err = user_update_coins(user)) != (SHARED_OK))
    {
        flight_book_error_handler(req, err);
        return_code = (KORE_RESULT_OK);
        goto exit;
    }

    Ticket ticket = {0, flightid, user->identifier, 250};
    if((err = ticket_insert(&ticket)))
    {
        flight_book_error_handler(req, err);
        return_code = (KORE_RESULT_OK);
        goto exit;
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK,
        asset_flight_book_success_html,
        asset_len_flight_book_success_html);
    return_code = (KORE_RESULT_OK);
exit:
    user_destroy(&user);
    return return_code;
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
        default:
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode, "/flight/search");
    }
}