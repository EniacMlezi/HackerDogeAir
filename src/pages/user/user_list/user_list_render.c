#include "pages/user/user_list/user_list_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/shared/shared_render.h"
#include "model/user.h"

int         user_list_render(UserListContext *);
void        user_list_render_clean(UserListContext *);
uintmax_t   user_list_varget(mustache_api_t *, void *, mustache_token_variable_t *);
uintmax_t   user_list_sectget(mustache_api_t *, void *, mustache_token_section_t *);

int
user_list_render(UserListContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &user_list_varget,
        .sectget = &user_list_sectget,
        .error = &shared_error,
    };

    if((err = shared_render((SharedContext *)context, &api, (const char* const)asset_user_list_chtml)) 
        != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void         
user_list_render_clean(UserListContext *context)
{
    shared_render_clean(&context->shared_context);
}

uintmax_t
user_list_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    UserContext *ctx = (UserContext *) userdata;
    const char *output_string = NULL;
    
    if(strncmp("id", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char id_string[11];
            if(snprintf(id_string, 11, "%d", ctx->user->id) <= 0)
            {
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = id_string;
        }
    }

    else if(strncmp("email", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user || NULL == ctx->user->email)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->user->email;
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "failed user list render: unknown template variable");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    size_t output_string_len = strlen(output_string);
    uintmax_t ret = api->write(api, userdata, output_string, output_string_len);
    if(ret != output_string_len)
    {
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}

uintmax_t
user_list_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token)
{
    UserListContext *ctx = (UserListContext *)userdata;

    if(strcmp("users", token->name) == 0)
    {
        UserListNode *user_node = NULL;
        api->write = &shared_mustache_strwrite;
        SLIST_FOREACH(user_node, &ctx->userlist, users)
        {
            // build a single user context foreach user.
            UserContext usercontext = {
                .dst_context = ctx->shared_context.dst_context,
                .user = &user_node->user
            };           
            if(!mustache_render(api, &usercontext, token->section))
            {
                api->write = &shared_strwrite;
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        api->write = &shared_strwrite;
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    return (SHARED_RENDER_MUSTACHE_FAIL);
}