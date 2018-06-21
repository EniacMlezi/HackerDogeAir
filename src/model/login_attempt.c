#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>
#include <stdbool.h>

#include <kore/kore.h>
#include <kore/pgsql.h>
#include <kore/http.h>

#include "model/login_attempt.h"
#include "model/user.h"
#include "model/database_engine.h"
#include "shared/shared_error.h"

static const char login_attempt_insert_query[] = 
    "INSERT INTO \"LoginAttempt\" (useridentifier,datetime,success) " \
    "VALUES ($1, now(), $2);";

static const char login_attempt_delete_query[] = 
    "DELETE FROM \"LoginAttempt\" WHERE useridentifier = $1;";

static const char login_attempt_retrieve_attempts_query[] = 
    "SELECT * FROM \"LoginAttempt\" " \
    "WHERE useridentifier = $1 " \
    "AND datetime >= CURRENT_TIMESTAMP AT TIME ZONE \'CEST\' - INTERVAL \'5 minutes\';";

//static const char login_attempt_select_by_identifier[] = "";

LoginAttempt *
login_attempt_create(uint32_t user_identifier, bool login_result, uint32_t *error)
{
    LoginAttempt *login_attempt = malloc(sizeof(LoginAttempt));

    if(login_attempt == NULL)
    {
        kore_log(LOG_ERR, "login_attempt_create: Could not allocate memory for a login_attempt structure.");
        *error = (LOGIN_ATTEMPT_ERROR_CREATE);
        return NULL;
    }

    login_attempt->user_identifier = user_identifier;
    login_attempt->login_result = login_result;

    *error = (SHARED_OK);
    return login_attempt;
}

void
login_attempt_destroy(LoginAttempt *login_attempt)
{
    free(login_attempt);
    login_attempt = NULL;
}

void *
login_attempt_create_from_query(void *source_location, uint32_t *error)
{
    if(kore_pgsql_nfields((struct kore_pgsql *) source_location) != 1)
    {
        kore_log(LOG_ERR, "login_attempt_create_from_query: Invalid source location");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    int err;

    uint32_t identifier = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 0), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "login_attempt_create_from_query: Could not translate db_identifier " \
            "string to uint32_t");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t success_integer = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 0), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "login_attempt_create_from_query: Could not translate db_success " \
            "string to uint32_t");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }    

    uint32_t create_login_attemp_result;
    void *temp_login_attempt = login_attempt_create(identifier, success_integer, 
        &create_login_attemp_result);

    if(temp_login_attempt == NULL)
    {
        kore_log(LOG_ERR, "login_attempt_create_from_query: Could not create a login_attempt " \
            "structure."); 
        *error = create_login_attemp_result;
    } 

    return temp_login_attempt;
}

uint32_t
login_attempt_collection_destroy(LoginAttempt *login_attempt_collection)
{
     TAILQ_HEAD(login_attempt_collection, LoginAttemptCollection) head;

     while(!TAILQ_EMPTY(&head))
     {
        LoginAttemptCollection *temp = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, temp, login_attempt_collection);

        login_attempt_destroy(temp->login_attempt);
        free(temp);
        temp = NULL;
     }

     return 0;
}

uint32_t
login_attempt_insert(const LoginAttempt *login_attempt)
{
    uint32_t user_identifier = htonl(login_attempt->user_identifier);

    uint32_t query_result = database_engine_execute_write(login_attempt_insert_query, 2,
    &user_identifier, sizeof(user_identifier), 1,
    &login_attempt->login_result, sizeof(login_attempt->login_result), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("login_attempt_insert", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

uint32_t
login_attempt_delete(LoginAttempt *login_attempt)
{
    uint32_t user_identifier = htonl(login_attempt->user_identifier);

    uint32_t query_result = database_engine_execute_write(login_attempt_delete_query, 1,
        &user_identifier, sizeof(user_identifier), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("login_attempt_delete", query_result);
        return query_result;
    }   

    return (SHARED_OK);
}

void *
login_attempt_get_amount_of_attempts(void *source_location, uint32_t *error)
{
#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    return (void *) kore_pgsql_ntuples((struct kore_pgsql *) source_location); 
#pragma GCC diagnostic pop
}

uint32_t
login_attempt_amount_of_logins_in_five_minutes(uint32_t user_identifier, uint32_t *error)
{
    *error = (SHARED_OK);
    uint32_t network_user_identifier = htonl(user_identifier);

    uint32_t query_result;
    void *data = database_engine_execute_read(login_attempt_retrieve_attempts_query, 
        login_attempt_get_amount_of_attempts, &query_result, 1,
        &network_user_identifier, sizeof(network_user_identifier), 1);

    if(data == NULL)
    {
        if(query_result == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            *error = query_result;
            return 0;
        }
        database_engine_log_error("login_attempt_amount_of_logins_in_five_minutes", query_result);  
        *error = (query_result);
    }
#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    return (uint32_t)data;
#pragma GCC diagnostic pop
}