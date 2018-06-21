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
     != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
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

    else if(strncmp("timeout", token->text, token->text_length) == 0)
    {
        char timeout_string[12];
        if(snprintf(timeout_string, 12, "%d", ctx->timeout) <= 0)
        {
            kore_log(LOG_ERR, 
                "error_varget: failed int to string conversion for timeout. input: %d",
                ctx->timeout);
            return (SHARED_RENDER_MUSTACHE_FAIL);
        }
        output_string = timeout_string;
    }

    if(NULL == output_string)
    {
        kore_log(LOG_ERR, "error_varget: unknown template variable '%s'", token->text);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    ctx->partial_context.should_html_escape = true;
    if(api->write(api, userdata, output_string, strlen(output_string)) != 
        (SHARED_RENDER_MUSTACHE_OK))
    {
        kore_log(LOG_ERR, "error_varget: failed to write");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}