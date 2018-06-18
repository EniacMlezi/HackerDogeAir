#include "pages/partial/header/header_render.h"

#include <stdio.h>
#include <kore/kore.h>
#include <mustache.h>

#include "shared/shared_error.h"
#include "assets.h"

int header_render(PartialContext *context);
void header_render_clean(PartialContext *context);
uintmax_t header_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
header_render(PartialContext *context)
{   
    int err = 0;

    if((err = partial_render_create_str_context(context, 
        (const char* const)asset_header_chtml)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    mustache_api_t api={
        .read = &partial_strread,  //std read will suffice
        .write = &partial_strwrite,     // need custom write for handling mustache_str_ctx **
        .varget = &header_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = partial_render_mustache_render(&api, context)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void 
header_render_clean(PartialContext *context)
{
    partial_render_clean(context);
}

uintmax_t
header_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    PartialContext *ctx = (PartialContext *)userdata;
    if(strncmp("session_id", token->text, token->text_length) == 0)
    {
        char session_id[12];
        if(snprintf(session_id, 12, "%d", ctx->session_id) <= 0)
        {
            return (SHARED_RENDER_MUSTACHE_FAIL); //error
        }
        size_t session_id_len = strlen(session_id);
        uintmax_t ret = api->write(api, userdata, session_id, session_id_len);
        if(ret != session_id_len)
        {
            return (SHARED_RENDER_MUSTACHE_FAIL);
        }
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    return (SHARED_RENDER_MUSTACHE_FAIL);
}