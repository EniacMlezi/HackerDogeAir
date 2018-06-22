#ifndef FLIGHTLIST_RENDER_H
#define FLIGHTLIST_RENDER_H

#include <mustache.h>
#include <sys/queue.h>

#include "pages/partial/partial_render.h"
#include "model/flight.h"

typedef struct FlightListNode
{
    Flight flight;
    SLIST_ENTRY(FlightSearchListNode) flights;
} FlightListNode;

typedef struct FlightListContext
{
    PartialContext partial_context;
    const char *error_message;
    SLIST_HEAD(head_s, FlightListNode) flightlist;
} FlightListContext;

int
admin_flight_list_render(FlightListContext *context);

void         
admin_flight_list_render_clean(FlightListContext *context);

#endif
