#ifndef USER_DETAIL_RENDER_H
#define USER_DETAIL_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>

#include "pages/shared/shared_render.h"
#include "model/user.h"

typedef struct UserDetailContext
{
    SharedContext shared_context;
    const char *error_message;
    User *user;
} UserDetailContext;

int
user_detail_render(UserDetailContext *context);

void         
user_detail_render_clean(UserDetailContext *context);

#endif