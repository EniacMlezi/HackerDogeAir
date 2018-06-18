#ifndef REGISTER_RENDER_H
#define REGISTER_RENDER_H

#include <mustache.h>
#include <stdbool.h>

#include "pages/partial/partial_render.h"
#include "model/user.h"

typedef struct RegisterContext
{
    PartialContext partial_context;
    const char *error_message;
    User *user;
} RegisterContext;

int
register_render(RegisterContext *context);

void         
register_render_clean(RegisterContext *context);

#endif