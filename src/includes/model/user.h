#ifndef USER_H
#define USER_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

#include "role.h"

typedef struct _User
{
    uint32_t    identifier;
    Role        role; 
    char        *email;
    char        *user_name;
    char        *password;
    double      doge_coin;
    char        *regristration_time; 
} User;

typedef struct UserCollection
{
    User *user;
    TAILQ_ENTRY(UserCollection) users;
} UserCollection;

User *
user_create(
    uint32_t identifier,
    Role    role,
    char    *user_name,
    char    *email,
    char    *password,
    double  doge_coin,
    char    *regristration_time
    );

void *
user_create_from_query(
    void *source_location
    );

void *
user_list_create_from_query(
    void *source_location
     );

uint32_t 
user_destroy(
    User *user
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
user_delete(
    User *user
    );

User *
user_find_by_email(
    const char *email
    );

User *
user_find_by_identifier(
    uint32_t identifier
    );

uint32_t
user_collection_destroy(
    UserCollection *user_collection
    );

UserCollection *
user_collection_get(
    void
    );

uint32_t
user_update_coins(
    User *user,
    double coins
    );
    
#endif