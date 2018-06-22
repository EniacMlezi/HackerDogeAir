#ifndef FLIGHTLIST_RENDER_H
#define FLIGHTLIST_RENDER_H

#include <mustache.h>
#include <sys/queue.h>

#include "pages/partial/partial_render.h"
#include "model/flight.h"

typedef struct FlightListContext
{
    PartialContext partial_context;
    const char *error_message;
    struct FlightCollection *flight_collection;
} FlightListContext;

int
admin_flight_list_render(FlightListContext *context);

void         
admin_flight_list_render_clean(FlightListContext *context);

#endif
