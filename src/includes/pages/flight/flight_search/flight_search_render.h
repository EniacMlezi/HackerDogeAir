#ifndef FLIGHT_SEARCH_RENDER_H
#define FLIGHT_SEARCH_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>
#include <time.h>

#include "pages/partial/partial_render.h"
#include "model/flight.h"

typedef struct FlightSearchParams
{
    struct tm arrivaldate;
} FlightSearchParams;


typedef struct FlightSearchContext
{
    PartialContext partial_context;
    const char *error_message;
    FlightSearchParams params;
    struct FlightCollection *flights;
} FlightSearchContext;

int
flight_search_render(FlightSearchContext *context);

void 
flight_search_render_clean(FlightSearchContext *context);

#endif