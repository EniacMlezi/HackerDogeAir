
#include "pages/login/login_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"

int         login_render(LoginContext *);
void        login_render_clean(LoginContext *);
uintmax_t   login_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token);

int
login_render(LoginContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &login_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_login_chtml))
     != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}


uintmax_t
login_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    LoginContext *ctx = (LoginContext *) userdata;
    const char *output_string = NULL;

    if(strncmp("login-lockout", token->text, token->text_length) == 0)
    {
        if(ctx->login_lockout)
        {
            output_string = "login-lockout";
        }
        else
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }

        ctx->user_context.partial_context.should_html_escape = true;
        if(api->write(api, userdata, output_string, strlen(output_string)) 
            != (SHARED_RENDER_MUSTACHE_OK))
        {
            kore_log(LOG_ERR, "login_varget: failed to write");
            return (SHARED_RENDER_MUSTACHE_FAIL);
        }
        return (SHARED_RENDER_MUSTACHE_OK);
    }

    return user_varget(api, userdata, token);
}

void
login_render_clean(LoginContext *context)
{
    partial_render_clean(&context->user_context.partial_context);
}