#include <stdbool.h>

#include <kore/kore.h>
#include <kore/http.h>

#include "shared/shared_error.h"

void    shared_error_handler(struct http_request *, int);
void    shared_error_response(struct http_request *, int, const char *);

void    
shared_error_handler(struct http_request *req, int errcode)
{
    switch(errcode)
    {
        case (SHARED_ERROR_SQL_DB_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Database connection failed.");
            break;
        case (SHARED_ERROR_SQL_QUERY_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Query failed.");
            break;
        case (SHARED_ERROR_SQL_RESULT_TRANSLATE_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Internal Server error. Query result parsing failed.");
            break;
        case (SHARED_ERROR_HASH_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error. Failed to produce a hash for given password.");

        default: 
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, "Internal Server error. " \
                 "Generic unhandled error.");
    }
}

void 
shared_error_response(struct http_request *req, int statuscode, const char *msg)
{
    http_response_header(req, "content-type", "text/plain");
    http_response(req, statuscode, (const void *)msg, strlen(msg));
}