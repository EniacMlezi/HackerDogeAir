#ifndef DELETEFLIGHT_RENDER_H
#define DELETEFLIGHT_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"

int
admin_delete_flight_render(PartialContext *context);

void         
admin_delete_flight_render_clean(PartialContext *context);

#endif