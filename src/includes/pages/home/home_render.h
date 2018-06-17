#ifndef HOME_RENDER_H
#define HOME_RENDER_H

#include <mustache.h>

#include "pages/shared/shared_render.h"

int
home_render(SharedContext *context);

void         
home_render_clean(SharedContext *context);

#endif