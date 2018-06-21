#ifndef PARTIAL_RENDER_H
#define PARTIAL_RENDER_H

#include <stdbool.h>
#include <mustache.h>

#define SHARED_RENDER_ERROR_TEMPLATE    1001
#define SHARED_RENDER_ERROR_RENDER      1002

#define SHARED_RENDER_MUSTACHE_FAIL     0
#define SHARED_RENDER_MUSTACHE_OK       !SHARED_RENDER_MUSTACHE_FAIL

extern const char* const SHARED_RENDER_EMPTY_STRING;
extern const char* const SHARED_RENDER_INVALID_STRING;
extern const char* const SHARED_RENDER_HIDDEN_STRING;

typedef struct PartialContext
{
    mustache_str_ctx *src_context;
    mustache_str_ctx *dst_context;
    bool should_html_escape; // set this to true before a partial_strwrite to enforce html escaping
    int session_id; //TODO: replace with a session struct
} PartialContext;

/*
Function for rendering a full page. First renders all partials for a template, then renders
the page specific elements for said tempalte using the supplied api.
*/
int
full_render(PartialContext *context, mustache_api_t *api, const char *const template);

/*
Calls the actual mustache functions for creating and rendering a template.
*/
int 
partial_render_mustache_render(mustache_api_t *api, void *context);

/*
Frees the string contexts for a given. PartialContext.
*/
void 
partial_render_clean(PartialContext *context);

/*
Creates a copy of a PartialContext, 
without copying over the string contexts(src_context, dst_context).
*/
void
partial_render_copy_context(PartialContext *src, PartialContext *dst);

/*
Creates a new set of string contexts (src_context, dst_context) for the
given PartialContext. The string contexts need to be freed by the caller after usage.
*/
int 
partial_render_create_str_context(PartialContext *context, const char* const template);

/*
Partial variable get funtion.
Replaces a PARTIAL_* token with the partial view, or rewrites unknown tags.
*/
uintmax_t
partial_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token);

/*
Partial mustache section get funtion.
Rewrites the original section begin and en tags and rewrites the section format.
*/
uintmax_t
partial_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token);

/*
Partial mustache read function.
Reads from a template and places it into PartialContext string contexts.
*/
uintmax_t 
partial_strread(mustache_api_t *api, void *userdata, char *buffer, uintmax_t buffer_size);

/*
Partial mustache write function.
Writes to a PartialContext string context.
*/
uintmax_t
partial_strwrite(mustache_api_t *api, void *userdata, char const *buffer, uintmax_t buffer_size);

/*
Partial mustache error handler.
Prints out any errors encountered by mustache.
*/
void
partial_error(mustache_api_t *api, void *userdata, uintmax_t lineno, char const *error);

#endif