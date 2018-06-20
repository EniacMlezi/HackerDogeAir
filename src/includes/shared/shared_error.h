#ifndef SHARED_ERROR_H
#define SHARED_ERROR_H

#include <kore/http.h>

#define SHARED_ERROR_OK                             0
#define SHARED_ERROR_SQL_DB_ERROR                   10
#define SHARED_ERROR_SQL_QUERY_ERROR                11
#define SHARED_ERROR_SQL_RESULT_TRANSLATE_ERROR     12
#define SHARED_ERROR_HASH_ERROR                     20

//generates a generic error response for given error.
void shared_error_handler(struct http_request *request, int status_code, const char *);
void shared_error_response(struct http_request *request,
    int status_code,
    const char *message,
    const char *redirect_uri,
    int timeout);

#endif