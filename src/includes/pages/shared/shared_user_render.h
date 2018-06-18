#ifndef SHARED_USER_RENDER_H
#define SHARED_USER_RENDER_H

#include "pages/partial/partial_render.h"
#include "model/user.h"

typedef struct UserContext
{
    PartialContext partial_context;
    const char *error_message;
    User *user;
} UserContext;

uintmax_t
user_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token);

#endif