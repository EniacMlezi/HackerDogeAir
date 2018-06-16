
#include "pages/login/login_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/shared/shared_render.h"
#include "model/user.h"

int         login_render(LoginContext *);
void        login_render_clean(LoginContext *);
uintmax_t   login_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
login_render(LoginContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &login_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    if((err = shared_render((SharedContext *)context, &api, (const char* const)asset_login_chtml))
     != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void
login_render_clean(LoginContext *context)
{
    shared_render_clean(&context->shared_context);
}

uintmax_t
login_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{   //custom varget works on UserContext.
    LoginContext *ctx = (LoginContext *) userdata;
    const char *output_string = NULL;
    if(strncmp("email", token->text, token->text_length) == 0)
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

    else if (strncmp("error_message", token->text, token->text_length) == 0)
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
        kore_log(LOG_INFO, "failed login render: unknown template variable");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    api->write(api, userdata, output_string, strlen(output_string));
    return (SHARED_RENDER_MUSTACHE_OK);
}