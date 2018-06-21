#ifndef SHARED_ERROR_H
#define SHARED_ERROR_H

#include <kore/http.h>

/* Shared error codes. */
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
#define USER_CREATE_SUCCESS                         51

/* Login_Attempt specific error codes. */
#define LOGIN_ATTEMPT_CREATE_SUCCESS                60
#define LOGIN_ATTEMPT_ERROR_CREATE                  61
#define LOGIN_ATTEMPT_ERROR_INSERT                  62

/* Logic specific error codes. */
#define REGISTER_ERROR_EMAIL_VALIDATOR_INVALID      202
#define REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID   203

#define SHARED_TIME_CONVERSION_OK                   251
#define SHARED_TIME_CONVERSION_ERROR                252

/* Login specific error codes. */
#define LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID        101
#define LOGIN_ERROR_EMAIL_VALIDATOR_INVALID         102
#define LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID      103
#define LOGIN_ERROR_EMAIL_INCORRECT                 104 
#define LOGIN_ERROR_PASSWORD_INCORRECT              105  
#define LOGIN_LOG_ATTEMPT_SUCCESS                   106
#define LOGIN_SUCCESSFULL_VALIDATION                107
#define LOGIN_SUCCESSFULL_LOGIN                     108
#define LOGIN_ERROR_INVALLID_CREDENTIALS            109
#define LOGIN_ERROR_LOG_ERROR                       110
#define LOGIN_BRUTEFORCE_CHECK_VALID                111


//generates a generic error response for given error.
void shared_error_handler(struct http_request *request, int status_code, const char *);
void shared_error_response(struct http_request *request,
    int status_code,
    const char *message,
    const char *redirect_uri,
    int timeout);

#endif