#ifndef USERS_RENDER_H
#define USERS_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>

#include "pages/shared/shared_render.h"
#include "model/user.h"

typedef struct UserNode
{
    User user;
    SLIST_ENTRY(UserNode) users;
} UserNode;

typedef struct UserContext
{   //UserContext type that can be written using mustache_std_strwrite
    mustache_str_ctx *dst_context;
    User *user;
} UserContext;

typedef struct UsersContext
{
    SharedContext shared_context;   //UsersContext inherits from SharedContext (castable)
    SLIST_HEAD(head_s, UserNode) userlist;
} UsersContext;

int
users_render(UsersContext *context);

void         
users_render_clean(UsersContext *context);

#endif