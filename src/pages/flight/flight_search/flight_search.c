#define _XOPEN_SOURCE 700 //must be defined for strptime from time.h
#include <stdbool.h>
#include <limits.h>
#include <mustache.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/flight/flight_search/flight_search_render.h"
#include "model/user.h"
#include "assets.h"
#include "time.h"

#define FLIGHT_SEARCH_ERROR_ARRIVALDATE_VALIDATOR_INVALID 401

int     flight_search(struct http_request *);
int     flight_search_parseparams(struct http_request *, FlightSearchParams *search_params);

void    flight_search_error_handler(struct http_request *, int, FlightSearchContext *);

int
flight_search(struct http_request *req)
{
    int err = 0;
    FlightSearchContext context = {
        .shared_context = {.session_id = 0},
        .params = {.arrivaldate = 0 }
    };
    SLIST_INIT(&context.flightlist);
    if(req->method == HTTP_METHOD_GET)
    {
        if((err = flight_search_render(&context)))
        {
            flight_search_error_handler(req, err, &context);
        }
        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK,
            context.shared_context.dst_context->string,
            strlen(context.shared_context.dst_context->string));
        flight_search_render_clean(&context);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {
        return (KORE_RESULT_ERROR);
    }

    if((err = flight_search_parseparams(req, &context.params)) != (SHARED_ERROR_OK))
    {
        flight_search_error_handler(req, err, &context);
        return (KORE_RESULT_OK);
    }

    //TODO: DataAccess layer search using context.params
    char *departure0 = "SchipInJeHol";
    char *arrival0 = "UitjeHol";
    time_t departuretime0 = time(NULL) + 10*60;
    time_t arrivaltime0 = time(NULL) + 50*60;
    FlightSearchListNode flight_node0 = {
        .flight = {
            .arrival_datetime = arrivaltime0,
            .departure_datetime = departuretime0,
            .arrival_location = arrival0,
            .departure_location = departure0
        }
    };
    SLIST_INSERT_HEAD(&context.flightlist, &flight_node0, flights);

    char *departure1 = "SchipUitJeHol";
    char *arrival1 = "InjeHol";
    time_t departuretime1 = time(NULL) + 10*60;
    time_t arrivaltime1 = time(NULL) + 50*60;
    FlightSearchListNode flight_node1 = {
        .flight = {
            .arrival_datetime = arrivaltime1,
            .departure_datetime = departuretime1,
            .arrival_location = arrival1,
            .departure_location = departure1
        }
    };
    SLIST_INSERT_HEAD(&context.flightlist, &flight_node1, flights);

    if((err = flight_search_render(&context)) != (SHARED_ERROR_OK))
    {
        flight_search_error_handler(req, err, &context);  
        return (KORE_RESULT_OK); 
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK,
        context.shared_context.dst_context->string,
        strlen(context.shared_context.dst_context->string));
    flight_search_render_clean(&context);
    return (KORE_RESULT_OK);
}

int flight_search_parseparams(struct http_request *req, FlightSearchParams *search_params)
{
    http_populate_post(req);
    char *date = NULL;
    if(!http_argument_get_string(req, "arrivaldate", &date))
    {
        return (FLIGHT_SEARCH_ERROR_ARRIVALDATE_VALIDATOR_INVALID);
    }

    struct tm date_tm;
    memset(&date_tm, 0, sizeof(date_tm));
    char *ret = strptime(date, "%d-%m-%Y", &date_tm);
    if(NULL == ret || *ret != '\0')
    {
        return (FLIGHT_SEARCH_ERROR_ARRIVALDATE_VALIDATOR_INVALID);
    }
    search_params->arrivaldate = mktime(&date_tm);
    return (SHARED_ERROR_OK);
}

void
flight_search_error_handler(struct http_request *req, int errcode, FlightSearchContext *context)
{
    kore_log(LOG_INFO, "flight search error: %d", errcode);
    shared_error_response(req, 500, "flight search error handler");
}
