#ifndef SHARED_RENDER_H
#define SHARED_RENDER_H

#include <mustache.h>

#define SHARED_RENDER_ERROR_ALLOC       1000
#define SHARED_RENDER_ERROR_TEMPLATE    1001
#define SHARED_RENDER_ERROR_RENDER      1002

#define SHARED_RENDER_MUSTACHE_FAIL     0
#define SHARED_RENDER_MUSTACHE_OK       !SHARED_RENDER_MUSTACHE_FAIL

extern const char* const SHARED_RENDER_EMPTY_STRING;
extern const char* const SHARED_RENDER_INVALID_STRING;

typedef struct SharedContext
{
    mustache_str_ctx *src_context;
    mustache_str_ctx *dst_context;
    int session_id; //TODO: replace with a session struct
} SharedContext;

/*
Shared function for rendering a page. First renders all shared partials for a template, then renders
the page specific elements for said tempalte using the supplied api.
*/
int
shared_render(SharedContext *context, mustache_api_t *api, const char* const template);

/*
Calls the actual mustache functions for creating and rendering a template.
*/
int 
shared_render_mustache_render(mustache_api_t *api, void *context);

/*
Frees the string contexts for a given. SharedContext.
*/
void 
shared_render_clean(SharedContext *context);

/*
Creates a copy of a SharedContext, 
without copying over the string contexts(src_context, dst_context).
*/
void
shared_render_copy_context(SharedContext *src, SharedContext *dst);

/*
Creates a new set of string contexts (src_context, dst_context) for the
given SharedContext. The string contexts need to be freed by the caller after usage.
*/
int 
shared_render_create_str_context(SharedContext *context, const char* const template);

uintmax_t
shared_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token);

uintmax_t 
shared_strread(mustache_api_t *api, void *userdata, char *buffer, uintmax_t buffer_size);

uintmax_t
shared_strwrite(mustache_api_t *api, void *userdata, char const *buffer, uintmax_t buffer_size);

void
shared_error(mustache_api_t *api, void *userdata, uintmax_t lineno, char const *error);

#endif