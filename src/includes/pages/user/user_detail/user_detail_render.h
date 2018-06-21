#ifndef USER_DETAIL_RENDER_H
#define USER_DETAIL_RENDER_H

#include <mustache.h>
#include <sys/queue.h>

#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"

int
user_detail_render(UserContext *context);

void         
user_detail_render_clean(UserContext *context);

#endif