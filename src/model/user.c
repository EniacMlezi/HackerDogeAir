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
#include "includes/model/database_engine.h"
#include "shared/shared_error.h"

static const char user_insert_query[] = 
    "INSERT INTO User (useridentifier,role,emailaddress,username,password,dogecoin," \
    "regristrationtime) VALUES ($1, $2, $3, $4, $5, $6, $7);";

static const char user_update_query[] =
    "UPDATE User " \
    "SET " \
    "useridentifier = $1, " \
    "role = $2, " \
    "emailaddress = $3 " \
    "username = $4" \
    "password = $5" \
    "dogecoin = $6" \
    "regristrationtime = $7" \
    "WHERE Identifier = $ 8;";

static const char user_delete_query[] = "DELETE FROM User WHERE useridentifier = $1;";

static const char user_select_by_email_query[] = 
    "SELECT (useridentifier, role, emailaddress, username, password ,dogecoin, regristrationtime) " \
    "FROM User " \
    "WHERE emailaddress = $1;";

static const char user_select_by_username_query[] = 
    "SELECT (useridentifier, role, emailaddress, username, password ,dogecoin, regristrationtime) " \
    "FROM User " \
    "WHERE username = $1;";

static const char user_select_by_identifier_query[] = 
    "SELECT (useridentifier, role, emailaddress, username, password ,dogecoin, regristrationtime) " \
    "FROM User " \
    "WHERE useridentifier = $1;";

User *
user_create(uint32_t identifier, Role role, char *user_name, char *email, char *password, 
    double coins, char *regristration_time, uint32_t *error)
{
    uint8_t name_size = strlen(user_name);
    uint8_t email_size = strlen(email);
    uint8_t password_size = strlen(password);
    uint8_t regristration_time_size = strlen(regristration_time);

    uint32_t user_size = name_size + email_size + password_size + regristration_time_size;

    /* Allocating space for the structure and the strings appended to it. */
    User *user = malloc(sizeof(User) + user_size); 
    if(user == NULL)
    {
        perror("user_create: Could not allocate memory for the user object.\n");
        *error = 1; /* Change this later to a more meaningfull error code. */
        return NULL;
    }

    uint32_t user_memory_offset = sizeof(User);

    /* Performing memory administration for the user structure. */
    user->user_name = (void *) user + user_memory_offset;
    user_memory_offset += name_size; 
    user->email = (void *) user + user_memory_offset;
    user_memory_offset += email_size;
    user->password = (void *) user + user_memory_offset;
    user_memory_offset += password_size; 
    user->regristration_datetime = (void *) user + user_memory_offset; 

    /* Initializing the user structure. */
    user->identifier = identifier; 
    user->role = role;
    user->doge_coin = coins;
    strncpy(user->email, email, email_size);
    strncpy(user->user_name, user_name, name_size);
    strncpy(user->password, password, password_size);

    if(regristration_time != NULL)
    {
        strncpy(user->regristration_datetime, regristration_time, regristration_time_size); 
    }

    return user;
}

void *
user_create_from_query(void *source_location, uint32_t *error)
{
    /* Determine if the first record of the query result contains the expected amount of fields. */
    /* Is most likely 6 in stead of 7 because of the 0nth element. */
    if(kore_pgsql_nfields((struct kore_pgsql *) source_location) != 6)
    {
        perror("user_create_from_query: Invallid source location.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL; 
    }

/*
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
*/

    char *user_name;
    char *email;
    char *password;
    char *regristration_time;

    user_name   = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 3);
    email       = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 2);
    password    = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 4);
    regristration_time = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 6);

    /* Potential conversion errors in the code block below, Testing must be performed to confirm the 
    proper behaviour of the following code segment. */
    int err;

    uint32_t identifier = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 0), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "user_create_from_query: Could not translate db_user_id string to " \
            "uint32_t.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    Role role = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 1), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "user_create_from_query: Could not translate db_user_role string to " \
            "uint64_t.\n");
        *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
        return NULL; 
    }

    double doge_coin = kore_strtodouble(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 5), 0, UINT_MAX, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "user_create_from_query: Could not translate db_user_coins string to " \
            "double.\n"); 
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t create_user_result;
    void *temp_user = user_create(identifier, role, email, user_name, password, doge_coin, 
        regristration_time, &create_user_result);

    if(temp_user == NULL)
    {
        perror("user_create_from_query: Could not create an user structure.\n");
        *error = create_user_result;
    }
    
    return temp_user;
}

void *
user_collection_create_from_query(void *source_location, uint32_t *error)
{
    uint32_t number_of_results = kore_pgsql_ntuples((struct kore_pgsql *) source_location);

    if(number_of_results <= 0)
    {
        perror("user_collection_create_from_query: Invallid number of record. Must be more " \
            "than zero.\n");
        *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
        return NULL;
    }

    TAILQ_HEAD(user_collection_s, UserCollection) *user_collection = malloc(sizeof(UserCollection));
    TAILQ_INIT(user_collection);


    uint32_t i;
    for(i =0; i < number_of_results; ++i)
    {
        User *temp_user = NULL;

/*
        uint8_t user_name_length;
        uint8_t email_length;
        uint8_t password_length;
        uint8_t regristration_time_length;

        user_name_length = kore_pgsql_getlength((struct kore_pgsql *) source_location, i, 3);
        email_length = kore_pgsql_getlength((struct kore_pgsql *) source_location, i, 2);
        password_length = kore_pgsql_getlength((struct kore_pgsql *) source_location, i, 4);
        regristration_time_length = kore_pgsql_getlength((struct kore_pgsql *) 
            source_location, i, 6);
*/

        char *user_name;
        char *email;
        char *password;
        char *regristration_time;

        user_name   = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 3);
        email       = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 2);
        password    = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 4);
        regristration_time = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 6);

        /* Potential conversion errors in the code block below, Testing must be performed to confirm the 
        proper behaviour of the following code segment. */
        int err;

        uint32_t identifier = kore_strtonum64(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 0), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "user_collection_create_from_query: Could not translate " \
                "db_user_id string to uint32_t.\n");
            *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
            return NULL;
        }

        Role role = kore_strtonum64(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 1), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "user_collection_create_from_query: Could not translate " \
                "db_user_role string to uint64_t.\n");
            *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
            return NULL; 
        }

        double doge_coin = kore_strtodouble(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 5), 0, UINT_MAX, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "user_collection_create_from_query: Could not translate " \
                "db_user_coins string to double.\n"); 
            *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
            return NULL;
        }

        uint32_t create_user_result;
        temp_user = user_create(identifier, role, user_name, email, password, doge_coin, 
            regristration_time, &create_user_result);

        if(temp_user == NULL)
        {
            perror("user_collection_create_from_query: Could not create an user structure.\n");
            *error = create_user_result;
            return NULL;
        }

        UserCollection *temp_collection = malloc(sizeof(UserCollection));
        temp_collection->user = temp_user;

        TAILQ_INSERT_TAIL(user_collection, temp_collection, user_collection);
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
user_collection_destroy(UserCollection *user_collection)
{
    TAILQ_HEAD(user_collection, UserCollection) head;

    while(!TAILQ_EMPTY(&head))
    {
        UserCollection *temp = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, temp, user_collection);

        free(temp->user);
        free(temp);
        temp = NULL;
    }

    return 0;
}

uint32_t
user_insert(const User *user)
{
    uint32_t error = database_engine_execute_write(user_insert_query, 7, 
        user->identifier,
        user->role,
        user->email,
        user->user_name,
        user->password,
        user->doge_coin,
        user->regristration_datetime); 

    switch(error)
    {
        case DATABASE_ENGINE_ERROR_INITIALIZATION:
        case DATABASE_ENGINE_ERROR_QUERY_ERROR:
        perror("user_insert: Could not insert user.\n");
        break;

        case DATABASE_ENGINE_OK:
        default:
        break;
    }

    return error;
}

uint32_t
user_update(const User *user)
{
    uint32_t error = database_engine_execute_write(user_update_query, 7, 
        user->identifier,
        user->role,
        user->email,
        user->user_name,
        user->password,
        user->doge_coin,
        user->regristration_datetime,
        user->identifier); 

    switch(error)
    {
        case DATABASE_ENGINE_ERROR_INITIALIZATION:
        case DATABASE_ENGINE_ERROR_QUERY_ERROR:
            perror("user_update: Could not update user.\n");
        break;

        case DATABASE_ENGINE_OK:
        default:
        break;
    }

    return error;
}

uint32_t
user_delete(User *user)
{
    uint32_t error = database_engine_execute_write(user_update_query, 7, 
        user->identifier);

    switch(error)
    {
        case DATABASE_ENGINE_ERROR_INITIALIZATION:
        case DATABASE_ENGINE_ERROR_QUERY_ERROR:
            perror("user_delete: Could not delete user.\n");
        break;

        case DATABASE_ENGINE_OK:
        default:
        break;
    }

    return error;
}

User *
user_find_by_email(const char *email, uint32_t *error)
{
    uint32_t query_error;
    void *result;

    result = database_engine_execute_read(user_select_by_email_query, user_create_from_query, 
        &query_error, 1, email);

    if(result == NULL)
    {
        switch(query_error)
        {
            case (DATABASE_ENGINE_ERROR_NO_RESULTS):
                perror("user_find_by_email: Could not find a user with the corresponding " \
                    "email address.\n");
                *error = query_error;
            break;

            case (DATABASE_ENGINE_ERROR_INITIALIZATION):
            case (DATABASE_ENGINE_ERROR_QUERY_ERROR):
            case (DATABASE_ENGINE_ERROR_RESULT_PARSE):
            default:
                perror("user_find_by_email: Could not find user.\n");
                *error = query_error;
            break;
        }
    }

    return result;
}

User *
user_find_by_user_name(const char *user_name, uint32_t *error)
{
    uint32_t query_error;
    void *result;

    result = database_engine_execute_read(user_select_by_email_query, user_create_from_query, 
        &query_error, 1, user_name);

    if(result == NULL)
    {
        switch(query_error)
        {
            case (DATABASE_ENGINE_ERROR_NO_RESULTS):
                perror("user_find_by_user_name: Could not find a user with the corresponding " \
                    "user name.\n");
                *error = query_error;
            break;

            case (DATABASE_ENGINE_ERROR_INITIALIZATION):
            case (DATABASE_ENGINE_ERROR_QUERY_ERROR):
            case (DATABASE_ENGINE_ERROR_RESULT_PARSE):
            default:
                perror("user_find_by_user_name: Could not find user.\n");
                *error = query_error;
            break;
        }
    }

    return result;
}

User *
user_find_by_identifier(uint32_t identifier, uint32_t *error)
{
    uint32_t query_error;
    void *result;

    result = database_engine_execute_read(user_select_by_email_query, user_create_from_query, 
        &query_error, 1, identifier);

    if(result == NULL)
    {
        switch(query_error)
        {
            case (DATABASE_ENGINE_ERROR_NO_RESULTS):
                perror("user_find_by_identifier: Could not find a user with the corresponding " \
                    "identifier.\n");
                *error = query_error;
            break;

            case (DATABASE_ENGINE_ERROR_INITIALIZATION):
            case (DATABASE_ENGINE_ERROR_QUERY_ERROR):
            case (DATABASE_ENGINE_ERROR_RESULT_PARSE):
            default:
                perror("user_find_by_identifier: Could not find a user.\n");
                *error = query_error;
            break;
        }
    }

    return result;
}
