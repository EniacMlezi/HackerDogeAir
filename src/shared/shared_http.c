#include <kore/kore.h>
#include <kore/http.h>

void    error_response(struct http_request *, int, const char *);

void    
error_response(struct http_request *req, int http_statuscode, const char *msg)
{
    http_response_header(req, "content-type", "text/plain");
    http_response(req, http_statuscode, (const void *)msg, strlen(msg));
}