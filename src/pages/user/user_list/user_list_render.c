#include "pages/user/user_list/user_list_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"

int         user_list_render(UserListContext *);
void        user_list_render_clean(UserListContext *);
uintmax_t   user_list_sectget(mustache_api_t *, void *, mustache_token_section_t *);

int
user_list_render(UserListContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &partial_varget,
        .sectget = &user_list_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_user_list_chtml)) 
        != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void         
user_list_render_clean(UserListContext *context)
{
    partial_render_clean(&context->partial_context);
}

uintmax_t
user_list_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token)
{
    UserListContext *ctx = (UserListContext *)userdata;

    if(strcmp("users", token->name) == 0)
    {
        UserListNode *user_node = NULL;
        api->varget = &user_varget;
        SLIST_FOREACH(user_node, &ctx->userlist, users)
        {
            // build a single user context foreach user.
            UserContext usercontext = {
                .user = &user_node->user
            };
            memcpy(&usercontext.partial_context, &ctx->partial_context, sizeof(PartialContext));          
            if(!mustache_render(api, &usercontext, token->section))
            {
                api->varget = &partial_varget;
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        api->varget = &partial_varget;
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    return (SHARED_RENDER_MUSTACHE_FAIL);
}