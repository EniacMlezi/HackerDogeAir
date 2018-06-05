#ifndef SHARED_ERROR_H
#define SHARED_ERROR_H

#define SHARED_ERROR_OK                             0
#define SHARED_ERROR_SQL_DB_ERROR                   10
#define SHARED_ERROR_SQL_QUERY_ERROR                11
#define SHARED_ERROR_SQL_RESULT_TRANSLATE_ERROR     12
#define SHARED_ERROR_HASH_ERROR                     20

#define DATABASE_ENGINE_NO_RESULTS

//generates a generic error response for given error.
void shared_error_handler(struct http_request *, int);
void shared_error_response(struct http_request *, int, const char *);
#endif