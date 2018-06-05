#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>

#include <kore/kore.h>
#include <kore/pgsql.h>
#include <kore/http.h>

#include "includes/model/user.h"
#include "includes/model/role.h"
#include <includes/model/database_engine.h>

User *
user_create(uint32_t identifier, Role role, char *user_name, char *email, char *password, 
    double coins, char *regristration_time)
{
    uint8_t name_size = strlen(user_name);
    uint8_t email_size = strlen(email);
    uint8_t password_size = strlen(password);
    uint8_t regristration_time_size = strlen(regristration_time);

    uint32_t user_size = name_size + email_size + password_size + regristration_time_size;

    /* Allocating space for the structure and the strings appended to it. */
    User *user = malloc(sizeof(User) + user_size); 

    uint32_t user_memory_offset = sizeof(User);

    /* Performing memory administration for the user structure. */
    user->user_name = (void *) user + user_memory_offset;
    user_memory_offset += name_size; 
    user->email = (void *) user + user_memory_offset;
    user_memory_offset += email_size;
    user->password = (void *) user + user_memory_offset;
    user_memory_offset += password_size; 
    user->regristration_time = (void *) user + user_memory_offset; 

    /* Initializing the user structure. */
    user->identifier = identifier; 
    user->role = role;
    user->doge_coin = coins;
    strncpy(user->email, email, email_size);
    strncpy(user->user_name, user_name, name_size);
    strncpy(user->password, password, password_size);

    if(regristration_time != NULL)
    {
        strncpy(user->regristration_time, regristration_time, regristration_time_size); 
    }

    return user;
}

void *
user_create_from_query(void *source_location)
{
    /* Determine if the first record of the query result contains the expected amount of fields. */
    /* Is most likely 6 in stead of 7 because of the 0nth element. */
    if(kore_pgsql_nfields((struct kore_pgsql *) source_location) != 6)
    {
        perror("user_create_from_query: Invallid source location.\n");
        return (void *) (DATABASE_ENGINE_ERROR_RESULT_PARSE);
    }

    uint8_t user_name_length;
    uint8_t email_length;
    uint8_t password_length;
    uint8_t regristration_time_length;

    user_name_length = kore_pgsql_getlength((struct kore_pgsql *) source_location, 0, 3);
    email_length = kore_pgsql_getlength((struct kore_pgsql *) source_location, 0, 2);
    password_length = kore_pgsql_getlength((struct kore_pgsql *) source_location, 0, 4);
    regristration_time_length = kore_pgsql_getlength((struct kore_pgsql *) source_location, 0, 6);

    char user_name[user_name_length];
    char email[email_length];
    char password[password_length];
    char regristration_time[regristration_time_length];

    strncpy(user_name, kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 3), 
        user_name_length);
    strncpy(email, kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 2), email_length);
    strncpy(password, kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 4), 
        password_length);
    strncpy(regristration_time, kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 6),
        regristration_time_length);

    /* Potential conversion errors in the code block below, Testing must be performed to confirm the 
    proper behaviour of the following code segment. */
    int error;

    uint32_t identifier = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 0), 0, &error);
    if(error != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "user_create_from_query: Could not translate db_user_id string to " \
            "uint32_t.");
        return (void *) (DATABASE_ENGINE_ERROR_RESULT_PARSE); 
    }

    Role role = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 1), 0, &error);
    if(error != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "user_create_from_query: Could not translate db_user_role string to " \
            "uint64_t.");
        return (void *) (DATABASE_ENGINE_ERROR_RESULT_PARSE);
    }

    double doge_coin = kore_strtodouble(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 5), 0, UINT_MAX, &error);
    if(error != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "user_create_from_query: Could not translate db_user_coins string to " \
            "double."); 
        return (void *) (DATABASE_ENGINE_ERROR_RESULT_PARSE);
    }

    return (void *) user_create(identifier, role, email, user_name, password, doge_coin, 
        regristration_time);
}

void *
user_collection_create_from_query(void *source_location)
{
    uint32_t number_of_results = kore_pgsql_ntuples((struct kore_pgsql *) source_location);

    if(number_of_results <= 0)
    {
        perror("user_list_create_from_query: Invallid number of record. Must be more than zero.");
        return (void *) (DATABASE_ENGINE_ERROR_NO_RESULTS);
    }

    TAILQ_HEAD(user_collection_s, UserCollection) *user_collection = malloc(sizeof(UserCollection));
    TAILQ_INIT(user_collection);


    uint32_t i;
    for(i =0; i < number_of_results; ++i)
    {
        User *temp_user = NULL;

        if(* ((int *) (temp_user = user_create_from_query(source_location))) == 
            (DATABASE_ENGINE_ERROR_RESULT_PARSE))
        {
            perror("user_list_create_from_query: Could not create a user from the " \
                " source_location.");
            return (void *) (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        }

        UserCollection *temp_collection = malloc(sizeof(UserCollection));
        temp_collection->user = temp_user;

        TAILQ_INSERT_TAIL(user_collection, temp_collection, users);
        temp_collection = NULL;
    }

    return (void *) user_collection;
}

uint32_t
user_destroy(User *user)
{
    free(user);
    user = NULL;

    return 0;
}

uint32_t
user_insert(const User *user)
{

}

uint32_t
user_update(const User *user)
{

}

uint32_t
user_delete(User *user)
{

}

User *
user_find_by_email(const char *email)
{

}

User *
user_find_by_identifier(uint32_t identifier)
{

}

uint32_t
user_collection_destroy(UserCollection *user_collection)
{

}

UserCollection *
user_collection_get(void)
{

}

uint32_t
user_update_coins(User *user, double coins)
{

}