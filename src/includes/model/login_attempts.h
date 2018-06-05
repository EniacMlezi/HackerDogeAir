#ifndef LOGIN_ATTEMPT_H
#define LOGIN_ATTEMPT_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct _LoginAttempt
{
    uint32_t    user_identifier;
    char        *login_time;
    bool        login_result;
} LoginAttempt;

typedef struct
{
    LoginAttempt login_attempt;
    LIST_ENTRY(_LoginAttempt) list;
} LoginAttemptList;

LoginAttempt *
login_attempt_create(
    uint32_t user_identifier,
    tm login_time,
    bool login_result
    );

uint32_t
login_attempt_destroy(
    LoginAttempt *login_attempt
    );

uint32_t
login_attempt_insert(
    const LoginAttempt *login_attempt
    );

uint32_t
login_attempt_update(
    const LoginAttempt *login_attempt
    );

uint32_t
login_attempt_delete(
    LoginAttempt *login_attempt
    );

LoginAttemptList *
login_attempt_list_create(
    void *source_location
    );

uint32_t
login_attempt_list_destroy(
    LoginAttempt *login_attempt_list
    );

LoginAttemptList *
login_attempt_list_find_by_identifier(
    uint32_t user_identifier
    );

#endif