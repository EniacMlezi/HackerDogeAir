#ifndef USER_BOOKINGS_RENDER_H
#define USER_BOOKINGS_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>
#include <time.h>

#include "pages/partial/partial_render.h"
#include "model/flight.h"

typedef struct UserBookingListNode
{
    Flight flight;
    SLIST_ENTRY(UserBookingListNode) userbookings;
} UserBookingListNode;

typedef struct UserBookingsContext
{
    PartialContext partial_context;
    const char *error_message;
    SLIST_HEAD(head_s, UserBookingListNode) userbookinglist;
} UserBookingsContext;

int
user_bookings_render(UserBookingsContext *context);

void
user_bookings_render_clean(UserBookingsContext *context);

#endif