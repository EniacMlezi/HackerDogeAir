#ifndef GIVEPOINTS_RENDER_H
#define GIVEPOINTS_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"

int
admin_give_points_render(UserContext *context);

void         
admin_give_points_render_clean(UserContext *context);

#endif