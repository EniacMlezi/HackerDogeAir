#include "pages/shared/shared_render.h"

#include <stdio.h>
#include <kore/kore.h>
#include <mustache.h>

#include "pages/shared/header/header_render.h"

int shared_render(SharedContext *, char *);
void shared_render_clean(SharedContext *context);
uintmax_t shared_varget(mustache_api_t *, void *, mustache_token_variable_t *);
uintmax_t shared_sectget(mustache_api_t *, void *, mustache_token_section_t *);
uintmax_t shared_strread(mustache_api_t *, void *, char *, uintmax_t);
uintmax_t shared_strwrite(mustache_api_t *, void *, char const *, uintmax_t);
void shared_error(mustache_api_t *, void *, uintmax_t, char const *);

int
shared_render(SharedContext *context, char *template_string)
{
    context->src_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));
    context->dst_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));

    context->src_context->string = template_string;
    context->src_context->offset = 0;
    context->dst_context->string = NULL;
    context->dst_context->offset = 0;

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &shared_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    mustache_template_t *template = mustache_compile(&api, context);
    mustache_render(&api, context, template);
    mustache_free(&api, template);

    return 0;
}

void
shared_render_clean(SharedContext *context)
{
    free(context->src_context);
    free(context->dst_context->string);
    free(context->dst_context);
}

uintmax_t
shared_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    int ret = 0;
    int err = 0;

    SharedContext *ctx = (SharedContext *)userdata;
    SharedContext new_ctx = *ctx; // copy?
    new_ctx.src_context = NULL; //just making sure
    new_ctx.dst_context = NULL; //just making sure

    if(strncmp("SHARED_HEADER", token->text, token->text_length) == 0)
    {
        header_render(&new_ctx);
        ret = api->write(api, userdata,
            new_ctx.dst_context->string, 
            strlen(new_ctx.dst_context->string));
        header_render_clean(&new_ctx);
        return ret;
    }

    //the found variable was not a shared partial view. rewrite the found var.
    //TODO: errors
    size_t length = token->text_length + 4; // 4 curly braces
    char *buffer = (char *)malloc(length);
    if((err = sprintf(buffer, "{{%s}}", token->text)) != length)
    {
        kore_log(LOG_INFO, "failed snprintf: %d", err);
        return 0; //FAIL
    }
    ret = api->write(api, userdata, buffer, length);
    free(buffer);
    kore_log(LOG_INFO, "var ret: %d", ret);
    return ret;
}

uintmax_t
shared_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token)
{
    return mustache_render(api, userdata, token->section);
}

uintmax_t
shared_strread(mustache_api_t *api, void *userdata, char *buffer, uintmax_t buffer_size)
{
    char *string;
    uintmax_t string_len;
    mustache_str_ctx *ctx = ((SharedContext *)userdata)->src_context; 
    
    string     = ctx->string + ctx->offset;
    string_len = strlen(string);
    string_len = (string_len < buffer_size) ? string_len : buffer_size;
    
    memcpy(buffer, string, string_len);
    
    ctx->offset += string_len;
    return string_len;
}

uintmax_t
shared_strwrite(mustache_api_t *api, void *userdata, char const *buffer, uintmax_t buffer_size)
{
    mustache_str_ctx *ctx = ((SharedContext *)userdata)->dst_context; 

    ctx->string = (char *)realloc(ctx->string, ctx->offset + buffer_size + 1);
    
    memcpy(ctx->string + ctx->offset, buffer, buffer_size);
    ctx->string[ctx->offset + buffer_size] = '\0';
    
    ctx->offset += buffer_size;
    return buffer_size;
}

void
shared_error(mustache_api_t *api, void *userdata, uintmax_t lineno, char const *error)
{
    fprintf(stderr, "error: %d: %s\n", (int)lineno, error);
}