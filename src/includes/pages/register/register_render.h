#ifndef REGISTER_RENDER_H
#define REGISTER_RENDER_H

#include <mustache.h>
#include <stdbool.h>

#include "pages/shared/shared_render.h"
#include "model/user.h"

typedef struct RegisterContext{
    SharedContext shared_context;   //RegisterContext inherits from SharedContext (castable)
    bool invalid_email;
    bool invalid_password;
    User *user;
} RegisterContext;

int
register_render(RegisterContext *context);

void         
register_render_clean(RegisterContext *context);

#endif