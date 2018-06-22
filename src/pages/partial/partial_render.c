#include "pages/partial/partial_render.h"

#include <stdio.h>
#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "shared/shared_error.h"
#include "shared/shared_escape.h"
#include "pages/partial/header/header_render.h"

int full_render(PartialContext *, mustache_api_t *, const char* const);
int partial_render_mustache_render(mustache_api_t *api, void *context);
void partial_render_clean(PartialContext *context);
void partial_render_copy_context(PartialContext *, PartialContext *);
int partial_render_create_str_context(PartialContext *, const char* const);
uintmax_t partial_varget(mustache_api_t *, void *, mustache_token_variable_t *);
uintmax_t partial_sectget(mustache_api_t *, void *, mustache_token_section_t *);
uintmax_t partial_strread(mustache_api_t *, void *, char *, uintmax_t);
uintmax_t partial_strwrite(mustache_api_t *, void *, char const *, uintmax_t);
void partial_error(mustache_api_t *, void *, uintmax_t, char const *);

const char* const SHARED_RENDER_EMPTY_STRING = "";
const char* const SHARED_RENDER_INVALID_STRING = "invalid";
const char* const SHARED_RENDER_HIDDEN_STRING = "hidden";

int
full_render(PartialContext *context, mustache_api_t *api, const char* const template_string)
{
    int err = 0;

    //do not escape html yet. Only set html_escape before rendering in a variable. 
    context->should_html_escape = false;

    //render all partials
    PartialContext copy_context;
    partial_render_copy_context(context, &copy_context);
    if((err = partial_render_create_str_context(&copy_context, template_string)) 
        != (SHARED_OK))
    {
        if(err == SHARED_ERROR_ALLOC_ERROR)
        {   // do not clean up when allocation failed.
            return err;
        }
        goto exit;
    }
    mustache_api_t partial_api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &partial_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };
    if((err = partial_render_mustache_render(&partial_api, &copy_context)) != (SHARED_OK))
    {
        goto exit;
    }
    
    //render all page specifics using supplied api
    if((err = partial_render_create_str_context(context,
     (const char* const)copy_context.dst_context->string)) != (SHARED_OK))
    {
        goto exit;
    }
    if((err = partial_render_mustache_render(api, context)) != (SHARED_OK))
    {
        goto exit;
    }

    err = (SHARED_OK);

exit:
    partial_render_clean(&copy_context);
    return err;
}

int
partial_render_mustache_render(mustache_api_t *api, void *context) 
{
    int err;

    mustache_template_t *template = mustache_compile(api, context);
    if(NULL == template)
    {
        return (SHARED_RENDER_ERROR_TEMPLATE);
    }
    if((err = mustache_render(api, context, template)) == (SHARED_RENDER_MUSTACHE_FAIL))
    {
        return (SHARED_RENDER_ERROR_RENDER);
    }
    mustache_free(api, template);

    return (SHARED_OK);
}

void
partial_render_clean(PartialContext *context)
{ 
    if(context == NULL)
    {
        return;
    }
    if(context->src_context != NULL)
    {
        free(context->src_context);
    }
    if(context->dst_context != NULL)
    {
        free(context->dst_context->string);
        free(context->dst_context);
    }
    context->src_context = NULL;
    context->dst_context = NULL;
}

void
partial_render_copy_context(PartialContext *src, PartialContext *dst)
{
    dst->session_id = src->session_id;
    dst->should_html_escape = src->should_html_escape;
    dst->src_context = NULL;
    dst->dst_context = NULL;
}

int 
partial_render_create_str_context(PartialContext *context, const char* const template)
{
    //TODO: benefits of using kore_alloc ?
    context->src_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));
    context->dst_context = (mustache_str_ctx *)malloc(sizeof(mustache_str_ctx));

    if(NULL == context->src_context || NULL == context->dst_context)
    {
        return (SHARED_ERROR_ALLOC_ERROR);
    }
#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wcast-qual"
    context->src_context->string = (char *)template;
#pragma GCC diagnostic pop
    context->src_context->offset = 0;
    context->dst_context->string = NULL;
    context->dst_context->offset = 0;

    return (SHARED_OK);
}

uintmax_t
partial_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    uintmax_t output_string_len = 0;
    int err = 0;
    PartialContext *ctx = (PartialContext *) userdata;
    PartialContext new_ctx;
    partial_render_copy_context(ctx, &new_ctx);

    if(strncmp("PARTIAL_HEADER", token->text, token->text_length) == 0)
    {
        if((err = header_render(&new_ctx)) != (SHARED_OK))
        {
            return (SHARED_RENDER_MUSTACHE_FAIL);
        }
        output_string_len = strlen(new_ctx.dst_context->string);
        if (api->write(api, userdata, new_ctx.dst_context->string, output_string_len) 
                != (SHARED_RENDER_MUSTACHE_OK))
        {
            kore_log(LOG_ERR, "partial_varget: failed to write");
            header_render_clean(&new_ctx);
            return (SHARED_RENDER_MUSTACHE_FAIL);
        }
        header_render_clean(&new_ctx);
        return (SHARED_RENDER_MUSTACHE_OK);
    }

    //the found variable was not a partial view. rewrite the found variable.
    int length = token->text_length + 4 + 1; // +4 => curly braces, +1 => \0
    char *buffer = (char *)malloc(length);
    if(NULL == buffer)
    {
        kore_log(LOG_ERR, "partial_varget: failed malloc for non-partial token");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    if((err = snprintf(buffer, length, "{{%s}}", token->text)) != length-1) //-1 => exclude \0
    {
        kore_log(LOG_ERR, 
            "partial_varget: failed snprintf for non-partial token. wrote: %d - expected: %d",
             err, length);
        free(buffer);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    if(api->write(api, userdata, buffer, length-1) != (SHARED_RENDER_MUSTACHE_OK)) //-1 => exclude \0
    {
        kore_log(LOG_ERR, "partial_varget: failed to write");
        free(buffer);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    free(buffer);
    return (SHARED_RENDER_MUSTACHE_OK);
}

uintmax_t
partial_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token)
{   // the partial sectget does nothing but rewriting the section tags and format
    int err = 0;
    int length = strlen(token->name) + 4 + 1 + 1; // +4 => curly braces, +1 => '#' OR '/', +1 => \0
    
    char *buffer = (char *)malloc(length);
    if(NULL == buffer)
    {
        kore_log(LOG_ERR, "partial_sectget: failed malloc for section");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    if((err = snprintf(buffer, length, "{{#%s}}", token->name)) != length-1) //-1 => exclude \0
    {
        kore_log(LOG_ERR, 
            "failed snprintf for section: printed: %d - expected: %d", err, length);
        free(buffer);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    if(api->write(api, userdata, buffer, length-1) != (SHARED_RENDER_MUSTACHE_OK))//-1 => exclude \0
    {
        kore_log(LOG_ERR, "partial_sectget: failed to write");
        free(buffer);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    if(mustache_render(api, userdata, token->section) != (SHARED_RENDER_MUSTACHE_OK))
    {
        kore_log(LOG_ERR, "partial_sectget: failed render of section.");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    if((err = snprintf(buffer, length, "{{/%s}}", token->name)) != length-1) //-1 => exclude \0
    {
        kore_log(LOG_ERR, 
            "partial_sectget: failed snprintf for section: wrote: %d - expected: %d", err, length);
        free(buffer);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    if(api->write(api, userdata, buffer, length-1) != (SHARED_RENDER_MUSTACHE_OK))//-1 => exclude \0
    {
        kore_log(LOG_ERR, "partial_sectget: failed to write");
        free(buffer);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}

uintmax_t
partial_strread(mustache_api_t *api, void *userdata, char *buffer, uintmax_t buffer_size)
{
    char *string;
    uintmax_t string_len;
    mustache_str_ctx *ctx = ((PartialContext *)userdata)->src_context; 
    
    string     = ctx->string + ctx->offset;
    string_len = strlen(string);
    if(string_len >= buffer_size)
    {
        string_len = buffer_size;
    }
    
    memcpy(buffer, string, string_len);
    
    ctx->offset += string_len;
    return string_len;
}

uintmax_t
partial_strwrite(mustache_api_t *api, void *userdata, char const *buffer, uintmax_t buffer_size)
{
    mustache_str_ctx *ctx = ((PartialContext *)userdata)->dst_context; 
    bool should_html_escape = ((PartialContext *)userdata)->should_html_escape;

    uint8_t *string_to_write = NULL;
    size_t string_to_write_len = 0;
    if(should_html_escape)
    {
        string_to_write_len =
            shared_escape_html(&string_to_write, (const uint8_t *)buffer, buffer_size);
    }
    else
    {
#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wcast-qual"
        string_to_write = (uint8_t *)buffer;
#pragma GCC diagnostic pop
        string_to_write_len = buffer_size;
    }

    ctx->string = (char *)realloc(ctx->string, ctx->offset + string_to_write_len + 1);
    
    memcpy(ctx->string + ctx->offset, string_to_write, string_to_write_len);
    ctx->string[ctx->offset + string_to_write_len] = '\0';
    
    ctx->offset += string_to_write_len;

    if(should_html_escape)
    {
        if(string_to_write_len > buffer_size)
        {
            kore_log(LOG_WARNING, 
            "Possible xss attempt detected. html escaped '%s' => '%s'", buffer, string_to_write);
            free(string_to_write);
        }
        ((PartialContext *)userdata)->should_html_escape = false;
    }

    return (SHARED_RENDER_MUSTACHE_OK);
}

void
partial_error(mustache_api_t *api, void *userdata, uintmax_t lineno, char const *error)
{
    kore_log(LOG_ERR, "mustache error: %d: %s\n", (int)lineno, error);
}