#ifndef SHARED_BOOKING_RENDER_H
#define SHARED_BOOKING_RENDER_H

#include "pages/partial/partial_render.h"
#include "model/ticket.h"

typedef struct BookingContext
{
    PartialContext partial_context;
    const char *error_message;
    Ticket *ticket;
} BookingContext;

uintmax_t
booking_varget(
    mustache_api_t *api, 
    void *userdate, 
    mustache_token_variable_t *token
    );
#endif