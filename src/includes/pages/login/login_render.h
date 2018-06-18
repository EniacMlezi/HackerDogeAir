#ifndef LOGIN_RENDER_H
#define LOGIN_RENDER_H

#include <mustache.h>
#include <stdbool.h>

#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"

int
login_render(UserContext *context);

void         
login_render_clean(UserContext *context);

#endif