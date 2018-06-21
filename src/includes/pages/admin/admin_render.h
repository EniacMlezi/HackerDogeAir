#ifndef ADMIN_RENDER_H
#define ADMIN_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"

int
admin_render(PartialContext *context);

void         
admin_render_clean(PartialContext *context);

#endif