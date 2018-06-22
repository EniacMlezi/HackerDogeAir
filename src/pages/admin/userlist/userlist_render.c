
#include "pages/admin/userlist/userlist_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"

int         admin_user_list_render(UserListContext *);
void        admin_user_list_render_clean(UserListContext *);
void        admin_user_list_error(mustache_api_t *, void *, uintmax_t, char const *);

uintmax_t   admin_user_list_sectget(mustache_api_t *, void *, mustache_token_section_t *);

int
admin_user_list_render(UserListContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &partial_varget,
        .sectget = &admin_user_list_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_userlist_chtml))
     != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void
admin_user_list_render_clean(UserListContext *context)
{
    partial_render_clean(&context->partial_context);
}

uintmax_t
admin_user_list_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token) 
{
    UserListContext *ctx = (UserListContext *)userdata;
    if (strcmp("userlist", token->name) == 0)
    {
        if (NULL == &ctx->userlist)
        { 
            return (SHARED_RENDER_MUSTACHE_OK);
        }

        UserCollection *user_node = NULL;
        api->varget = &user_varget;
        UserContext usercontext;
        memcpy(&usercontext.partial_context, &ctx->partial_context, sizeof(PartialContext));
        TAILQ_FOREACH(user_node, &ctx->userlist, user_collection)
        {
            usercontext.user = &user_node->user;
            if(!mustache_render(api, &usercontext, token->section))
            {
                kore_log(LOG_ERR, "admin_user_list_sectget: failed to render a user");
                api->varget = &partial_varget;
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        api->varget = &partial_varget;
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    kore_log(LOG_ERR, "admin_user_list_sectget: unknown template section '%s'", token->name);
    return (SHARED_RENDER_MUSTACHE_FAIL);
}