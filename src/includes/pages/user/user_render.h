#ifndef USER_RENDER_H
#define USER_RENDER_H

#include "pages/partial/partial_render.h"

int
user_render(PartialContext *context);

void         
user_render_clean(PartialContext *context);

#endif