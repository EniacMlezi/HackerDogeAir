#ifndef SHARED_ERROR_H
#define SHARED_ERROR_H

#define SHARED_ERROR_OK                             0
#define SHARED_ERROR_SQL_DB_ERROR                   10
#define SHARED_ERROR_SQL_QUERY_ERROR                11
#define SHARED_ERROR_SQL_RESULT_TRANSLATE_ERROR     12
#define SHARED_ERROR_HASH_ERROR                     20

/* Database access specific error codes. */
#define DATABASE_ENGINE_OK                          40
#define DATABASE_ENGINE_ERROR_NO_RESULTS            41
#define DATABASE_ENGINE_ERROR_INITIALIZATION        42
#define DATABASE_ENGINE_ERROR_QUERY_ERROR           43
#define DATABASE_ENGINE_ERROR_INVALLID_RESULT       44
#define DATABASE_ENGINE_ERROR_RESULT_PARSE          45

/* Data model specific error codes. */
#define USER_CREATE_ERROR                           50

#define DATABASE_ENGINE_NO_RESULTS

//generates a generic error response for given error.
void shared_error_handler(struct http_request *, int);
void shared_error_response(struct http_request *, int, const char *);
#endif