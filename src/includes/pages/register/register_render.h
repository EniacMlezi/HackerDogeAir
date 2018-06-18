#ifndef REGISTER_RENDER_H
#define REGISTER_RENDER_H

#include <mustache.h>
#include <stdbool.h>

#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"

int
register_render(UserContext *context);

void         
register_render_clean(UserContext *context);

#endif