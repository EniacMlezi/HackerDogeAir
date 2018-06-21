#ifndef HOME_RENDER_H
#define HOME_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"

int
home_render(PartialContext *context);

void         
home_render_clean(PartialContext *context);

#endif