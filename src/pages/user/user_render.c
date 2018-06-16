#include "pages/user/user_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/shared/shared_render.h"
#include "model/user.h"

int         user_render(UserContext *);
void        user_render_clean(UserContext *);
uintmax_t   user_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
user_render(UserContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &user_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    if((err = shared_render((SharedContext *)context, &api, (const char* const)asset_user_chtml)) 
        != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void         
user_render_clean(UserContext *context)
{
    shared_render_clean(&context->shared_context);
}

uintmax_t
user_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
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
            char id_string[12];
            if(snprintf(id_string, 12, "%d", ctx->user->id) <= 0)
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
        kore_log(LOG_INFO, "failed user render: unknown template variable");
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