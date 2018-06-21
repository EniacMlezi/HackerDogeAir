#ifndef GIVEPOINTS_RENDER_H
#define GIVEPOINTS_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"

int
admin_give_points_render(PartialContext *context);

void         
admin_give_points_render_clean(PartialContext *context);

#endif