
#include "pages/register/register_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "model/user.h"

int         register_render(RegisterContext *);
void        register_render_clean(RegisterContext *);
uintmax_t   register_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
register_render(RegisterContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &register_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_register_chtml)) 
        != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void
register_render_clean(RegisterContext *context)
{
    partial_render_clean(&context->partial_context);
}

uintmax_t
register_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    RegisterContext *ctx = (RegisterContext *) userdata;
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
        kore_log(LOG_INFO, "failed register render: unknown template variable");
        return (SHARED_RENDER_MUSTACHE_FAIL); // unknown variable
    }

    size_t output_string_len = strlen(output_string);
    uintmax_t ret = api->write(api, userdata, output_string, output_string_len);
    if(ret != output_string_len)
    {
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}