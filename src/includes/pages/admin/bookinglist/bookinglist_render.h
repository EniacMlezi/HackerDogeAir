#ifndef BOOKINGLIST_RENDER_H
#define BOOKINGLIST_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"
#include "model/ticket.h"

typedef struct BookingListContext
{
    PartialContext partial_context;
    const char *error_message;
    struct TicketCollection *ticket_collection;
} BookingListContext;

int
admin_booking_list_render(BookingListContext *context);

void         
admin_booking_list_render_clean(BookingListContext *context);

#endif