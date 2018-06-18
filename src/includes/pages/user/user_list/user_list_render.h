#ifndef USER_LIST_RENDER_H
#define USER_LIST_RENDER_H

#include <mustache.h>
#include <sys/queue.h>
#include <stdbool.h>

#include "pages/partial/partial_render.h"
#include "model/user.h"

typedef struct UserListNode
{
    User user;
    SLIST_ENTRY(UserListNode) users;
} UserListNode;

typedef struct UserListContext
{
    PartialContext partial_context; //UserListContext inherits from PartialContext (castable)
    SLIST_HEAD(head_s, UserListNode) userlist;
} UserListContext;

int
user_list_render(UserListContext *context);

void         
user_list_render_clean(UserListContext *context);

#endif