#ifndef LOGIN_RENDER_H
#define LOGIN_RENDER_H

#include <mustache.h>
#include <stdbool.h>

#include "pages/shared/shared_render.h"
#include "model/user.h"

typedef struct LoginContext
{
    SharedContext shared_context;   //LoginContext inherits from SharedContext (castable)
    const char *error_message;
    User *user;
} LoginContext;

int
login_render(LoginContext *context);

void         
login_render_clean(LoginContext *context);

#endif