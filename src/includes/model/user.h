#ifndef USER_H
#define USER_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

#include "role.h"

typedef struct
{
    uint32_t    identifier;
    Role        role; 
    char        *email;
    char        *username;
    char        *first_name;
    char        *last_name;
    char        *telephone_number;
    char        *password;
    uint32_t     doge_coin;
    struct tm   registration_datetime; 
} User;

typedef struct UserCollection
{
    User *user;
    TAILQ_ENTRY(UserCollection) user_collection;
} UserCollection;

User *
user_create(
    uint32_t identifier,
    Role    role,
    const char    *username,
    const char    *email,
    const char    *user_first_name,
    const char    *user_last_name,
    const char    *telephone_number,
    const char    *password,
    uint32_t       doge_coin,
    struct tm      registration_datetime,
    uint32_t *error
    );

void *
user_create_from_query(
    void *source_location,
    uint32_t *error
    );

void *
user_collection_create_from_query(
    void *source_location,
    uint32_t *error
     );

void
user_destroy(
    User *user
    );

uint32_t
user_collection_destroy(
    UserCollection *user_collection
    );

uint32_t 
user_insert(
    const User *user
    );

uint32_t
user_update(
    const User *user
    );

uint32_t
user_update_details(
    const User *user
    );

uint32_t
user_delete(
    User *user
    );

User *
user_find_by_email(
    const char *email,
    uint32_t *error
    );

User *
user_find_by_username_or_email(
    const char *email,
    uint32_t *error
    );

User *
user_find_by_user_name(
    const char *user_name,
    uint32_t *error
    );

User *
user_find_by_session_identifier(
    const char *session_identifier,
    uint32_t *error
    );

User *
user_find_by_identifier(
    uint32_t identifier,
    uint32_t *error
    );

#endif