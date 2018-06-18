#include "pages/user/user_detail/user_detail_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "model/user.h"

int         user_detail_render(UserDetailContext *);
void        user_detail_render_clean(UserDetailContext *);
uintmax_t   user_detail_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
user_detail_render(UserDetailContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &user_detail_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    if((err = shared_render((SharedContext *)context, &api, (const char* const)asset_user_detail_chtml)) 
        != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void         
user_detail_render_clean(UserDetailContext *context)
{
    shared_render_clean(&context->shared_context);
}

uintmax_t
user_detail_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    UserDetailContext *ctx = (UserDetailContext *) userdata;
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

    else if(strncmp("error_message", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->error_message)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->error_message;
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