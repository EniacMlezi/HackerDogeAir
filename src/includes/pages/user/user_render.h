#ifndef USER_RENDER_H
#define USER_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>

#include "pages/shared/shared_render.h"
#include "model/user.h"

typedef struct UserContext
{
    SharedContext shared_context;
    const char *error_message;
    User *user;
} UserContext;

int
user_render(UserContext *context);

void         
user_render_clean(UserContext *context);

#endif