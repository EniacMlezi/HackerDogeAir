#ifndef SHARED_ERROR_H
#define SHARED_ERROR_H

#include <kore/http.h>

/* Shared error codes. */
#define SHARED_OK                                   0

/* Database access specific error codes. */
#define DATABASE_ENGINE_ERROR_NO_RESULTS            11
#define DATABASE_ENGINE_ERROR_INITIALIZATION        12
#define DATABASE_ENGINE_ERROR_QUERY_ERROR           13
#define DATABASE_ENGINE_ERROR_RESULT_PARSE          14

/*Miscellaneous error codes.*/
#define SHARED_ERROR_HASH_ERROR                     20
#define SHARED_ERROR_ALLOC_ERROR                    21
#define SHARED_ERROR_TIME_CONVERSION                22
#define SHARED_ERROR_COOKIE_NOT_FOUND               23
#define SHARED_ERROR_SESSION_NOT_FOUND              24

/* Session specific error codes. */
#define SESSION_ERROR_CREATE                        40

/* User specific error codes. */
#define USER_ERROR_CREATE                           50

/* Login_Attempt specific error codes. */
#define LOGIN_ATTEMPT_ERROR_CREATE                  60

/* Logic specific error codes. */
#define LOGIN_ERROR_EMAIL_VALIDATOR_INVALID         101
#define LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID      102
#define LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID        103
#define LOGIN_ERROR_EMAIL_INCORRECT                 104
#define LOGIN_ERROR_PASSWORD_INCORRECT              105
#define LOGIN_ERROR_LOG_ATTEMPT_ERROR               106

/* Register specific error codes. */
#define REGISTER_ERROR_EMAIL_VALIDATOR_INVALID      201
#define REGISTER_ERROR_PASSWORD_VALIDATOR_INVALID   202
#define REGISTER_ERROR_FIRSTNAME_VALIDATOR_INVALID  203
#define REGISTER_ERROR_LASTNAME_VALIDATOR_INVALID   204
#define REGISTER_ERROR_TELNUMBER_VALIDATOR_INVALID  205
#define REGISTER_ERROR_USERNAME_VALIDATOR_INVALID   206

/* Flight Book specific error codes. */
#define FLIGHT_BOOK_ERROR_ID_VALIDATOR_INVALID      400
#define FLIGHT_BOOK_ERROR_INSUFFICIENT_FUNDS        401

/* Admin domain specific error codes. */
#define ADMIN_DELETE_FLIGHT_ID_INVALID              501
#define ADMIN_DELETE_FLIGHT_ERROR                   502
#define ADMIN_GIVE_POINTS_ID_INVALID                503
#define ADMIN_GIVE_POINTS_ID_NOT_FOUND              504
#define ADMIN_GIVE_POINTS_INTEGER_CONVERSION_ERROR  505

//generates a generic error response for given error.
void shared_error_handler(struct http_request *request, int status_code, const char *redirect_uri);
void shared_error_response(struct http_request *request,
    int status_code,
    const char *message,
    const char *redirect_uri,
    int timeout);

#endif