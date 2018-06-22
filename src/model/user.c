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
#include "shared/shared_time.h"

static const char user_insert_query[] =
    "INSERT INTO \"User\" (userrole,emailaddress,username,password,dogecoin," \
    "registrationtime,firstname,lastname,telephonenumber) " \
    " VALUES ($1, $2, $3, $4, $5, now(), $6, $7, $8);";

static const char user_update_query[] =
    "UPDATE \"User\" " \
    "SET " \
    "useridentifier = $1, " \
    "userrole = $2, " \
    "emailaddress = $3," \
    "username = $4," \
    "password = $5," \
    "dogecoin = $6," \
    "firstname = $7," \
    "lastname = $8," \
    "telephonenumber = $9" \
    "WHERE useridentifier = $1;";

static const char user_update_details_query[] = 
    "UPDATE \"User\" SET " \
    "username = $2, " \
    "emailaddress = $3, " \
    "firstname = $4, " \
    "lastname = $5, " \
    "telephonenumber = $6 " \
    "WHERE useridentifier = $1;";

static const char user_delete_query[] = "DELETE FROM \"User\" WHERE useridentifier = $1;";

static const char user_select_by_identifier_query[] = 
    "SELECT useridentifier,userrole,emailaddress,username,password,dogecoin,registrationtime," \
    "firstname,lastname,telephonenumber " \
    "FROM \"User\" " \
    "WHERE useridentifier = $1;";

static const char user_select_by_session[] = 
    "SELECT useridentifier,userrole,emailaddress,username,password,dogecoin,registrationtime," \
    "firstname,lastname,telephonenumber FROM \"User\" WHERE useridentifier = " \
    "(SELECT useridentifier FROM \"Session\" WHERE sessionidentifier = $1);";

static const char user_select_by_email_or_username[] =
    "SELECT useridentifier,userrole,emailaddress,username,password,dogecoin,registrationtime," \
    "firstname,lastname,telephonenumber " \
    "FROM \"User\" " \
    "WHERE emailaddress = $1 OR username = $1;";

User *
user_create(uint32_t identifier, Role role, const char *username, const char *email, 
    const char *first_name, const char *last_name, const char *telephone_number, 
    const char *password, uint32_t doge_coins, struct tm registration_datetime, uint32_t *error)
{
    uint8_t name_size = strlen(username) + 1;
    uint8_t email_size = strlen(email) + 1;
    uint8_t password_size = strlen(password) + 1;
    uint8_t first_name_size = strlen(first_name) + 1;
    uint8_t last_name_size = strlen(last_name) + 1;
    uint8_t telephone_number_size = strlen(telephone_number) + 1;

    uint32_t user_size = name_size + email_size + password_size + first_name_size + 
    last_name_size + telephone_number_size;

    /* Allocating space for the structure and the strings appended to it. */
    User *user = malloc(sizeof(User) + user_size); 
    if(user == NULL)
    {
        perror("user_create: Could not allocate memory for the user object.\n");
        *error = (SHARED_ERROR_ALLOC_ERROR);
        return NULL;
    }

    uint32_t user_memory_offset = sizeof(User);

    #pragma GCC diagnostic push  // require GCC 4.6
    #pragma GCC diagnostic ignored "-Wpointer-arith"
 
    /* Performing memory administration for the user structure. */
    user->username = (void *) user + user_memory_offset;
    user_memory_offset += name_size; 
    user->email = (void *) user + user_memory_offset;
    user_memory_offset += email_size;
    user->password = (void *) user + user_memory_offset;
    user_memory_offset += password_size; 
    user->first_name = (void *) user + user_memory_offset;
    user_memory_offset += first_name_size;
    user->last_name = (void *) user + user_memory_offset;
    user_memory_offset += last_name_size;
    user->telephone_number = (void *) user + user_memory_offset;

    #pragma GCC diagnostic pop

    /* Initializing the user structure. */
    user->identifier = identifier; 
    user->role = role;
    user->doge_coin = doge_coins;
    strncpy(user->email, email, email_size);
    strncpy(user->username, username, name_size);
    strncpy(user->password, password, password_size);
    strncpy(user->first_name, first_name, first_name_size);
    strncpy(user->last_name, last_name, last_name_size);
    strncpy(user->telephone_number, telephone_number, telephone_number_size);
    user->registration_datetime = registration_datetime;

    return user;
}

void *
user_create_from_query(void *source_location, uint32_t *error)
{
    /* Determine if the first record of the query result contains the expected amount of fields. */
    /* Is most likely 6 in stead of 7 because of the 0nth element. */
    if(kore_pgsql_nfields((struct kore_pgsql *) source_location) != 10)
    {
        perror("user_create_from_query: Invalid source location.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL; 
    }

    char *user_name   = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 3);
    char *email       = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 2);
    char *password    = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 4);
    char *user_first_name = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 7);
    char *user_last_name = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 8);
    char *telephone_number = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 9);

    char *registration_time = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 6);
    struct tm registration_datetime;
    shared_time_database_string_to_tm(registration_time, &registration_datetime);

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
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL; 
    }

    uint32_t doge_coin = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 5), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "user_create_from_query: Could not translate db_user_coins string to " \
            "uint64_t.\n"); 
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t create_user_result;
    User *temp_user = user_create(identifier, role, user_name, email, user_first_name, 
        user_last_name, telephone_number, password, doge_coin, registration_datetime, 
        &create_user_result);

    if(temp_user == NULL)
    {
        perror("user_create_from_query: Could not create a user structure.\n");
        *error = create_user_result;
        return NULL;
    }

    *error = (SHARED_OK);
    return temp_user;
}

void *
user_collection_create_from_query(void *source_location, uint32_t *error)
{
    uint32_t number_of_results = kore_pgsql_ntuples((struct kore_pgsql *) source_location);

    TAILQ_HEAD(user_collection_s, UserCollection) *user_collection = malloc(sizeof(UserCollection));
    TAILQ_INIT(user_collection);

    uint32_t i;
    for(i =0; i < number_of_results; ++i)
    {
        User *temp_user = NULL;

        char *user_name   = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 3);
        char *email       = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 2);
        char *password    = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 4);
        char *user_first_name = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 7);
        char *user_last_name = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 8);
        char *telephone_number = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 9);

        char *registration_time = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 6);
        struct tm registration_datetime;
        shared_time_database_string_to_tm(registration_time, &registration_datetime);

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

        double doge_coin = kore_strtonum64(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 5), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "user_collection_create_from_query: Could not translate " \
                "db_user_coins string to double.\n"); 
            *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
            return NULL;
        }

        uint32_t create_user_result;
        temp_user = user_create(identifier, role, user_name, email, user_first_name, user_last_name, 
            telephone_number, password, doge_coin, registration_datetime, &create_user_result);

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

void
user_destroy(User *user)
{
    free(user);
    user = NULL;
}

uint32_t
user_collection_destroy(UserCollection *user_collection)
{
    TAILQ_HEAD(user_collection, UserCollection) head;

    while(!TAILQ_EMPTY(&head))
    {
        UserCollection *temp = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, temp, user_collection);

        user_destroy(temp->user);
        free(temp);
        temp = NULL;
    }

    return 0;
}

uint32_t
user_insert(const User *user)
{
    uint32_t role = htonl(user->role);
    uint32_t doge_coin = htonl(user->doge_coin);

    uint32_t query_result = database_engine_execute_write(user_insert_query, 8,
        &role, sizeof(role), 1,
        user->email, strlen(user->email), 0,
        user->username, strlen(user->username), 0,
        user->password, strlen(user->password), 0,
        &doge_coin, sizeof(doge_coin), 1,
        user->first_name, strlen(user->first_name), 0,
        user->last_name, strlen(user->last_name), 0,
        user->telephone_number, strlen(user->telephone_number), 0);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("user_insert", query_result);
        return query_result;
    }
    
    return (SHARED_OK);
}

uint32_t
user_update(const User *user)
{
    uint32_t identifier = htonl(user->identifier);
    uint32_t role = htonl(user->role);
    uint32_t doge_coin = htonl(user->doge_coin);

    uint32_t query_result = database_engine_execute_write(user_update_query, 9, 
        &identifier, sizeof(identifier), 1,
        &role, sizeof(role), 1,
        user->email, strlen(user->email), 0,
        user->username, strlen(user->username), 0,
        user->password, strlen(user->password), 0,
        &doge_coin, sizeof(doge_coin), 1,
        user->first_name, strlen(user->first_name), 0,
        user->last_name, strlen(user->last_name), 0,
        user->telephone_number, strlen(user->telephone_number), 0);

    if (query_result != (SHARED_OK))
    {
        database_engine_log_error("user_update", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

uint32_t
user_update_details(const User *user)
{
    uint32_t identifier = htonl(user->identifier);

    uint32_t query_result = database_engine_execute_write(user_update_details_query, 6,
        &identifier, sizeof(identifier), 1,
        user->username, strlen(user->username), 0,
        user->email, strlen(user->email), 0,
        user->first_name, strlen(user->first_name), 0,
        user->last_name, strlen(user->last_name), 0,
        user->telephone_number, strlen(user->telephone_number), 0);
    
    if (query_result != (SHARED_OK))
    {
        database_engine_log_error("user_update", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

uint32_t
user_delete(User *user)
{
    uint32_t identifier = htonl(user->identifier);

    uint32_t query_result = database_engine_execute_write(user_delete_query, 1, 
        &identifier, sizeof(identifier), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("user_delete", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

User *
user_find_by_session_identifier(const char *session_identifier, uint32_t *error)
{
    uint32_t query_result;
    void *result;

    result = database_engine_execute_read(user_select_by_session, user_create_from_query,
        &query_result, 1, session_identifier, strlen(session_identifier), 0);

    if(result == NULL)
    {
        // No results is a special case that does not require logging
        if(query_result == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            *error = query_result;
            return result;
        }
        database_engine_log_error("user_find_by_session_identifier", query_result);
        *error = query_result;
    }

    return result; 
}

User *
user_find_by_username_or_email(const char *email, uint32_t *error)
{
    uint32_t query_result;
    void *result;

    result = database_engine_execute_read(user_select_by_email_or_username, 
        &user_create_from_query, &query_result, 1, email, strlen(email), 0);

    if(result == NULL)
    {
        // No results is a special case that does not require logging
        if(query_result == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            *error = query_result;
            return result;
        }
        database_engine_log_error("user_find_by_username_or_email", query_result);
        *error = query_result;
    }

    return result; 
}

User *
user_find_by_identifier(uint32_t identifier, uint32_t *error)
{
    uint32_t query_result;
    void *result;

    uint32_t cast_identifier = htonl(identifier);

    result = database_engine_execute_read(user_select_by_identifier_query, user_create_from_query, 
        &query_result, 1, &cast_identifier, sizeof(cast_identifier), 1);

    if(result == NULL)
    {
        database_engine_log_error("user_find_by_identifier", query_result);
        *error = query_result;
    }

    return result;
}
