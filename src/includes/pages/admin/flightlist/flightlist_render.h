#ifndef FLIGHTLIST_RENDER_H
#define FLIGHTLIST_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"

int
admin_flight_list_render(PartialContext *context);

void         
admin_flight_list_render_clean(PartialContext *context);

#endif
