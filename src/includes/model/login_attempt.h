#ifndef LOGIN_ATTEMPT_H
#define LOGIN_ATTEMPT_H

#include <stdint.h>
#include <time.h>
#include <sys/queue.h>

typedef struct
{
    uint32_t    user_identifier;
    struct tm   login_attempt_date_time;
    bool        login_result;
} LoginAttempt;

typedef struct LoginAttemptCollection
{
    LoginAttempt *login_attempt;
    TAILQ_ENTRY(LoginAttemptCollection) login_attempt_collection;
} LoginAttemptCollection;

LoginAttempt *
login_attempt_create(
    uint32_t user_identifier,
    bool login_result,
    struct tm login_attempt_date_time,
    uint32_t *error
    );

void
login_attempt_destroy(
    LoginAttempt *login_attempt
    );

void *
login_attempt_create_from_query(
    void *source_location, 
    uint32_t *error
    );

void *
login_attempt_get_amount_of_attempts(
    void *source_location, 
    uint32_t *error
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

uint32_t
login_attempt_amount_of_logins_in_five_minutes(
    uint32_t user_identifier,
    uint32_t *error
    );

/*
LoginAttemptCollection *
login_attempt_collection_find_by_identifier(
    uint32_t user_identifier,
    uint32_t *error
    );
*/

#endif