#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>

#include <kore/kore.h>
#include <kore/pgsql.h>
#include <kore/http.h>

#include "includes/model/session.h"
#include "includes/model/database_engine.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"

static const char session_insert_or_update_query[] = 
    "INSERT INTO \"Session\" VALUES ($1, $2, $3) " \
    "ON CONFLICT (useridentifier) DO UPDATE " \
    "SET sessionidentifier = Excluded.sessionidentifier, validuntil = Excluded.validuntil;";

static const char session_delete_query[] = 
    "DELETE FROM \"Session\" WHERE sessionidentifier = $1;";

static const char session_select_by_session_identifier[] = 
    "SELECT sessionidentifier, useridentifier, validuntil FROM \"Session\" " \
    "WHERE sessionidentifier = $1 ORDER BY sessionidentifier ASC;";

static const char session_update_query[] = 
    "UPDATE \"Session\" SET sessionidentifier = $1, useridentifier = $2, validuntil = $3 " \
    "WHERE sessionidentifier = $1;"; 

Session *
session_create(char *session_identifier, uint32_t user_identifier, struct tm *expiration_time,
    uint32_t *error)
{
    uint32_t session_identifier_len = strlen(session_identifier) + 1;

    Session *session = malloc(sizeof(Session) + session_identifier_len);
    if(session == NULL)
    {
        kore_log(LOG_ERR, "session_create: Could not allocate memory for a session structure.\n");
        *error = (SHARED_ERROR_ALLOC_ERROR);
        return NULL;
    }

    uint32_t memory_offset = sizeof(Session);
    session->identifier = (char *)((char *)session + memory_offset);
    memory_offset += session_identifier_len;

    strncpy(session->identifier, session_identifier, session_identifier_len);
    session->user_identifier = user_identifier;
    session->expiration_time = *expiration_time;

    *error = (SHARED_OK);
    return session;
}

void *
session_create_from_query(void *source_location, uint32_t *error)
{
    int err = 0;
    struct kore_pgsql *pgsql = (struct kore_pgsql *)source_location;
    if(kore_pgsql_nfields(pgsql) != 3)
    {
        kore_log(LOG_ERR, "session_create_from_query: Invallid source location.");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    char *session_identifier = kore_pgsql_getvalue(pgsql, 0, 0);

    uint32_t user_identifier = kore_strtonum64(kore_pgsql_getvalue(pgsql, 0, 1), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "session_create_from_query: Could not translate db_user_identifier " \
            "string to uint32_t.");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    char *temp_experation_time = kore_pgsql_getvalue(pgsql, 0, 2);
    struct tm experation_time;
    if((err = shared_time_database_string_to_tm(temp_experation_time, &experation_time))
        != (SHARED_OK))
    {
        kore_log(LOG_ERR, "session_create_from_query: Time conversion error for experation_time");
        *error = err;
        return NULL;
    }

    uint32_t create_session_result = 0;
    void *temp_session = session_create(session_identifier, user_identifier, &experation_time, 
        &create_session_result);

    if(temp_session == NULL)
    {
        kore_log(LOG_ERR, "session_create_from_query: Could not create a session structure.");
        *error = create_session_result;
    }

    return temp_session;
}

void
session_destroy(Session **session)
{
    free(*session);
    *session = NULL;
}

uint32_t
session_insert_or_update(const Session *session)
{
    uint32_t error = 0;
    uint32_t user_identifier = htonl(session->user_identifier);

    char expiration_time[30]; //TODO: find appropriate size
    shared_time_tm_to_database_string(&session->expiration_time, expiration_time, 
        sizeof(expiration_time), &error);
    if(error != (SHARED_OK))
    {
        kore_log(LOG_ERR, "session_insert: Time conversion error. error: %d", error);
        return error;
    }

    uint8_t query_result = database_engine_execute_write(session_insert_or_update_query, 3,
        session->identifier, strlen(session->identifier), 0,
        &user_identifier, sizeof(user_identifier), 1,
        expiration_time, strlen(expiration_time), 0);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("session_insert", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

uint32_t
session_update(const Session *session)
{
    uint32_t error = 0;
    uint32_t user_identifier = htonl(session->user_identifier);
    
    char expiration_time[30]; //TODO: find appropriate size
    shared_time_tm_to_database_string(&session->expiration_time, expiration_time, 
        sizeof(expiration_time), &error);
    if(error != (SHARED_OK))
    {
        kore_log(LOG_ERR, "session_update: Time conversion error. error: %d", error);
        return error;
    }

    uint8_t query_result = database_engine_execute_write(session_update_query, 3,
        session->identifier, strlen(session->identifier), 0,
        &user_identifier, sizeof(user_identifier), 1,
        expiration_time, strlen(expiration_time), 0);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("session_update", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

uint32_t
session_delete(Session *session)
{
    uint8_t query_result = database_engine_execute_write(session_delete_query, 1,
        session->identifier, strlen(session->identifier), 0);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("session_delete", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

Session *
session_find_by_session_identifier(const char *session_identifier, uint32_t *error)
{
    uint32_t query_result = 0;
    void *result;

    result = database_engine_execute_read(session_select_by_session_identifier,
        session_create_from_query, &query_result, 1, 
        session_identifier, strlen(session_identifier), 0);

    if(result == NULL)
    {
        if(query_result == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            *error = query_result;
            return result;
        }

        database_engine_log_error("session_find_by_session_identifier", query_result);
        *error = query_result;
    }

    return result;
}
