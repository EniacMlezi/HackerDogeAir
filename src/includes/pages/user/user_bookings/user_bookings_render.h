#ifndef USER_BOOKINGS_RENDER_H
#define USER_BOOKINGS_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>
#include <time.h>

#include "pages/partial/partial_render.h"
//#include "model/flight.h"
#include "model/ticket.h"

typedef struct UserBookingsContext
{
    PartialContext partial_context;
    const char *error_message;
    struct TicketCollection *ticket_collection;
} UserBookingsContext;

int
user_bookings_render(UserBookingsContext *context);

void
user_bookings_render_clean(UserBookingsContext *context);

#endif