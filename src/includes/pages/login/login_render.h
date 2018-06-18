#ifndef LOGIN_RENDER_H
#define LOGIN_RENDER_H

#include <mustache.h>
#include <stdbool.h>

#include "pages/partial/partial_render.h"
#include "model/user.h"

typedef struct LoginContext
{
    PartialContext partial_context; //LoginContext inherits from PartialContext (castable)
    const char *error_message;
    User *user;
} LoginContext;

int
login_render(LoginContext *context);

void         
login_render_clean(LoginContext *context);

#endif