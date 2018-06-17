#ifndef LOGIN_ATTEMPT_H
#define LOGIN_ATTEMPT_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct
{
    uint32_t    user_identifier;
    char        *login_time;
    bool        login_result;
} LoginAttempt;

typedef struct LoginAttemptCollection
{
    LoginAttempt login_attempt;
    LIST_ENTRY(LoginAttemptCollection) login_attempt_collection;
} LoginAttemptCollection;

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
login_attempt_collection_destroy(
    LoginAttempt *LoginAttemptCollection
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

LoginAttemptCollection *
login_attempt_collection_create(
    void *source_location,
    uint32_t *error
    );

LoginAttemptCollection *
login_attempt_collection_find_by_identifier(
    uint32_t user_identifier,
    uint32_t *error
    );

#endif