#include "pages/shared/header/header_render.h"

#include <stdio.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"

int header_render(SharedContext *context);
void header_render_clean(SharedContext *context);
uintmax_t header_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
header_render(SharedContext *context)
{   
    context->src_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));
    context->dst_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));

    context->src_context->string = asset_header_chtml;
    context->src_context->offset = 0;
    context->dst_context->string = NULL;
    context->dst_context->offset = 0;

    mustache_api_t api={
        .read = &mustache_std_strread,  //std read will suffice
        .write = &shared_strwrite,     // need custom write for handling mustache_str_ctx **
        .varget = &header_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    mustache_template_t *template = mustache_compile(&api, context->src_context);
    mustache_render(&api, context, template);
    mustache_free(&api, template);

    return 0;
}

void header_render_clean(SharedContext *context)
{
    free(context->src_context);
    free(context->dst_context->string);
    free(context->dst_context);
}

uintmax_t
header_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    SharedContext *ctx = (SharedContext *)userdata;
    if(strncmp("session_id", token->text, token->text_length) == 0)
    {
        char session_id[11];
        if(snprintf(session_id, 11, "%d", ctx->session_id) <= 0)
        {
            return 0; //error
        }
        return api->write(api, userdata, session_id, strlen(session_id));
    }
    return 0; //error
}