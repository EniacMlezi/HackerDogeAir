
#include "pages/register/register_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/shared/shared_render.h"
#include "model/user.h"

int         register_render(RegisterContext *);
void        register_render_clean(RegisterContext *);
uintmax_t   register_varget(mustache_api_t *, void *, mustache_token_variable_t *);
void        register_error(mustache_api_t *, void *, uintmax_t, char const *);

int
register_render(RegisterContext *context)
{
    SharedContext new_ctx = *((SharedContext *)context); // copy?
    shared_render(&new_ctx, asset_register_chtml);

    context->shared_context.src_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));
    context->shared_context.dst_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));

    context->shared_context.src_context->string = new_ctx.dst_context->string;
    context->shared_context.src_context->offset = 0;
    context->shared_context.dst_context->string = NULL;
    context->shared_context.dst_context->offset = 0;

    mustache_api_t api={
        .read = &shared_strread,  //std read will suffice
        .write = &shared_strwrite,     // need custom write for handling mustache_str_ctx **
        .varget = &register_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    mustache_template_t *template = mustache_compile(&api, context);
    mustache_render(&api, context, template);
    mustache_free(&api, template);

    shared_render_clean(&new_ctx);

    return (SHARED_ERROR_OK);
}

void
register_render_clean(RegisterContext *context)
{
    free(context->shared_context.src_context);
    free(context->shared_context.dst_context->string);
    free(context->shared_context.dst_context);
}

uintmax_t
register_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{   //custom varget works on UserContext.
    RegisterContext *ctx = (RegisterContext *) userdata;
    if(strncmp("email", token->text, token->text_length) == 0)
    {
        if(ctx->user == NULL || ctx->user->email == NULL){
            return 1;
        }
        return api->write(api, userdata, ctx->user->email, strlen(ctx->user->email));
    }

    if (strncmp("invalid_email", token->text, token->text_length) == 0)
    {
        char *invalid_email_class = ctx->invalid_email ? "invalid" : NULL;
        if(invalid_email_class == NULL)
            return 1;
        return api->write(api, userdata, invalid_email_class, strlen(invalid_email_class));
    }

    if(strncmp("invalid_password", token->text, token->text_length) == 0)
    {
        char *invalid_password_class = ctx->invalid_password ? "invalid" : NULL;
        if(invalid_password_class == NULL)
            return 1;
        return api->write(api, userdata, invalid_password_class, strlen(invalid_password_class));
    }

    return 0; // error
}