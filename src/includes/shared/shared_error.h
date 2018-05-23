#ifndef SHARED_ERROR_H
#define SHARED_ERROR_H

#define OK                              0
#define SQL_DB_ERROR                    10
#define SQL_QUERY_ERROR                 11
#define SQL_RESULT_TRANSLATE_ERROR      12
#define HASH_ERROR                      20

//generates a generic error response for given error.
void shared_error_handler(struct http_request *, int);
void shared_error_response(struct http_request *, int, const char *);
#endif