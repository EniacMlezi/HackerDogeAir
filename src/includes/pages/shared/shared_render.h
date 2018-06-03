#ifndef SHARED_RENDER_H
#define SHARED_RENDER_H

#include <mustache.h>
#include <string.h>
#include <stdlib.h>

typedef struct SharedContext
{
    mustache_str_ctx *src_context;
    mustache_str_ctx *dst_context;
    int session_id; //TODO: replace with a session struct
} SharedContext;

int
shared_render(SharedContext *context, char *template);

void 
shared_render_clean(SharedContext *context);

uintmax_t
shared_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token);

uintmax_t 
shared_strread(mustache_api_t *api, void *userdata, char *buffer, uintmax_t buffer_size);

uintmax_t
shared_strwrite(mustache_api_t *api, void *userdata, char const *buffer, uintmax_t buffer_size);

void
shared_error(mustache_api_t *api, void *userdata, uintmax_t lineno, char const *error);

#endif