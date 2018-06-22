#ifndef BOOKINGLIST_RENDER_H
#define BOOKINGLIST_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"

int
admin_booking_list_render(PartialContext *context);

void         
admin_booking_list_render_clean(PartialContext *context);

#endif