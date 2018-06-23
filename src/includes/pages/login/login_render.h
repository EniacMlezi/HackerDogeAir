#ifndef LOGIN_RENDER_H
#define LOGIN_RENDER_H

#include <mustache.h>
#include <stdbool.h>

#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"

typedef struct LoginContext {
    UserContext user_context;
    bool login_lockout;
} LoginContext;

int
login_render(LoginContext *context);

void         
login_render_clean(LoginContext *context);

#endif