#ifndef SHARED_FLIGHT_RENDER_H
#define SHARED_FLIGHT_RENDER_H

#include "pages/partial/partial_render.h"
#include "model/flight.h"

typedef struct FlightContext
{
    PartialContext partial_context;
    const char *error_message;
    Flight *flight;
} FlightContext;

uintmax_t
flight_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token);

#endif