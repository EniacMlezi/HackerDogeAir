#include "pages/home/home_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/error/error_render.h"

int         error_render(ErrorContext *);
void        error_render_clean(ErrorContext *);
uintmax_t   error_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token);

int
error_render(ErrorContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &error_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_error_chtml))
     != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void
error_render_clean(ErrorContext *context)
{
    partial_render_clean((PartialContext *)context);
}

uintmax_t
error_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    ErrorContext *ctx = (ErrorContext *) userdata;
    const char *output_string = NULL;
    if(strncmp("error_message", token->text, token->text_length) == 0)
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

    else if(strncmp("redirect_uri", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->redirect_uri)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->redirect_uri;
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_ERR, "failed error render: unknown template variable '%s'", token->text);
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