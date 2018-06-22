#ifndef USERLIST_RENDER_H
#define USERLIST_RENDER_H

#include <mustache.h>
#include <sys/queue.h>

#include "pages/partial/partial_render.h"
#include "model/user.h"



typedef struct UserListContext
{
    PartialContext partial_context;
    const char *error_message;
    TAILQ_HEAD(head_s, UserCollection) userlist;
} UserListContext;

int
admin_user_list_render(UserListContext *context);

void         
admin_user_list_render_clean(UserListContext *context);

#endif