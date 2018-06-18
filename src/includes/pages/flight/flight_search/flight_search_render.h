#ifndef FLIGHT_SEARCH_RENDER_H
#define FLIGHT_SEARCH_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>
#include <time.h>

#include "pages/partial/partial_render.h"
#include "model/user.h"

typedef struct Flight
{   //TODO: should move to model with introduction of DataAccess layer
    int id;
    time_t arrival_datetime;
    time_t departure_datetime;
    char *arrival_location;
    char *departure_location;
} Flight;

typedef struct FlightContext
{   //FlightContext type that can be written to using shared_mustache_strwrite
    mustache_str_ctx *dst_context;
    Flight *flight;
} FlightContext;

typedef struct FlightSearchParams
{
    time_t arrivaldate;
} FlightSearchParams;

typedef struct FlightSearchListNode
{
    Flight flight;
    SLIST_ENTRY(FlightSearchListNode) flights;
} FlightSearchListNode;

typedef struct FlightSearchContext
{
    PartialContext partial_context;
    const char *error_message;
    FlightSearchParams params;
    SLIST_HEAD(head_s, FlightSearchListNode) flightlist;
} FlightSearchContext;

int
flight_search_render(FlightSearchContext *context);

void 
flight_search_render_clean(FlightSearchContext *context);

#endif