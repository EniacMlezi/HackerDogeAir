#ifndef BOOKINGLIST_RENDER_H
#define BOOKINGLIST_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"
#include "model/ticket.h"

typedef struct BookingContext
{
    PartialContext partial_context;
    const char *error_message;
    Ticket *booking;
} BookingContext;

typedef struct BookingListNode
{
    Ticket booking;
    SLIST_ENTRY(BookingListNode) bookings;
} BookingListNode;

typedef struct BookingListContext
{
    PartialContext partial_context;
    const char *error_message;
    SLIST_HEAD(head_s, BookingListNode) bookinglist;
} BookingListContext;

int
admin_booking_list_render(BookingListContext *context);

void         
admin_booking_list_render_clean(BookingListContext *context);

#endif