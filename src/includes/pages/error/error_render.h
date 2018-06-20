#ifndef ERROR_RENDER_H
#define ERROR_RENDER_H

#include "pages/partial/partial_render.h"

typedef struct ErrorContext {
    PartialContext partial_context;
    const char *error_message;
    const char *redirect_uri;
} ErrorContext;

int
error_render(ErrorContext *context);

void         
error_render_clean(ErrorContext *context);

#endif