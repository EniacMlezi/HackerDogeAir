
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
void        login_error(mustache_api_t *, void *, uintmax_t, char const *);

int
login_render(LoginContext *context)
{
    int err = 0;

    SharedContext new_ctx;
    shared_render_copy_context(&context->shared_context, &new_ctx);
    if((err = shared_render(&new_ctx, (const char* const)asset_login_chtml)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    if((err = shared_render_create_str_context(&context->shared_context,
     (const char* const)new_ctx.dst_context->string)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &login_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    if((err = shared_render_mustache_render(&api, context)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    shared_render_clean(&new_ctx);

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
        return 0;   //FAIL. unknown variable
    }

    api->write(api, userdata, output_string, strlen(output_string));
    return 1; //NO FAIL. written data.
}