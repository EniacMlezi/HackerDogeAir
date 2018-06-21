#ifndef USERLIST_RENDER_H
#define USERLIST_RENDER_H

#include <mustache.h>

#include "pages/partial/partial_render.h"

int
admin_user_list_render(PartialContext *context);

void         
admin_user_list_render_clean(PartialContext *context);

#endif