#include <stdbool.h>

#include <kore/kore.h>
#include <kore/http.h>

#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/error/error_render.h"

void    shared_error_handler(struct http_request *, int, const char *);
void    shared_error_response(struct http_request *, int, const char *, const char *, int timeout);

void    
shared_error_handler(struct http_request *req, int errcode, const char *redirect_uri)
{
    switch(errcode)
    {
        case (DATABASE_ENGINE_ERROR_INITIALIZATION):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Database connection failed.", redirect_uri, 5);
            break;
        case (DATABASE_ENGINE_ERROR_QUERY_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Query failed.", redirect_uri, 5);
            break;
        case (DATABASE_ENGINE_ERROR_RESULT_PARSE):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Internal Server error. Query result parsing failed.", redirect_uri, 5);
            break;
        case (SHARED_ERROR_HASH_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Failed to produce a hash for given password.", redirect_uri, 5);
        case (SHARED_ERROR_TIME_CONVERSION):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Internal Server error. Time conversion failed.", redirect_uri, 5);
        case (SHARED_RENDER_ERROR_TEMPLATE):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Bad template.", redirect_uri, 5);
            break;
        case (SHARED_RENDER_ERROR_RENDER):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Failed to render template.", redirect_uri, 5);
            break;
        case (SHARED_ERROR_ALLOC_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Render allocation failure.", redirect_uri, 5);
            break;

        default:
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Internal Server error. Generic unhandled error.", redirect_uri, 5);
    }
}

void 
shared_error_response(
    struct http_request *req,
    int statuscode,
    const char *msg,
    const char *redirect,
    int timeout)
{
    int err;

    ErrorContext context = {
        .error_message = msg,
        .redirect_uri = redirect,
        .timeout = timeout
    };
    if((err = error_render(&context)) != (SHARED_OK))
    {   // could not render error page. give the error plain text
        kore_log(LOG_ERR, "shared_error_response: failed to render error page.");
        http_response_header(req, "content-type", "text/plain");
        http_response(req, statuscode, (const void *)msg, strlen(msg));
    }
    http_response_header(req, "content-type", "text/html");
    http_response(req, statuscode,
        context.partial_context.dst_context->string,
        strlen(context.partial_context.dst_context->string));
}