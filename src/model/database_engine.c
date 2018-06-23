#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "model/database_engine.h"
#include "shared/shared_error.h"

static uint32_t
database_engine_initialize(
    struct kore_pgsql *pgsql,
    char *database_name
    );

static uint32_t
database_engine_close_connection(
    struct kore_pgsql *pgsql
    );

static uint32_t
database_engine_initialize(struct kore_pgsql *pgsql, char *database_name)
{
    uint32_t return_code;

    kore_pgsql_init(pgsql);
    if(!kore_pgsql_setup(pgsql, database_name, KORE_PGSQL_SYNC))
    {
        kore_pgsql_logerror(pgsql);
        return_code = (DATABASE_ENGINE_ERROR_INITIALIZATION);
        goto error_exit;
    }

    return (SHARED_OK);

    /* Error exit label. */
error_exit: 
    kore_pgsql_cleanup(pgsql);
    return return_code;
}

static uint32_t
database_engine_close_connection(struct kore_pgsql *pgsql)
{
    kore_pgsql_cleanup(pgsql); 
    return (SHARED_OK);
}

uint32_t
database_engine_execute_write(const char *sql_query, uint32_t count, ...)
{
    va_list parameters;
    va_start(parameters, count);

    uint32_t return_code = (SHARED_OK);
    struct kore_pgsql database_connection;

    if(database_engine_initialize(&database_connection, DATABASE_NAME) != (SHARED_OK))
    {
        return_code = (DATABASE_ENGINE_ERROR_INITIALIZATION);
        goto error_exit; 
    }    

    /* Execute the query on the connected database. */
    if(kore_pgsql_v_query_params(&database_connection, sql_query, 0, count, parameters) !=
        (KORE_RESULT_OK))
    {
        kore_pgsql_logerror(&database_connection);
        return_code = (DATABASE_ENGINE_ERROR_QUERY_ERROR);
        goto error_exit;
    }

error_exit:
    database_engine_close_connection(&database_connection);
    va_end(parameters);
    return return_code;
}

void *
database_engine_execute_read(const char *sql_query, 
    void *(*model_build_from_query)(void *source_location, uint32_t *error), uint32_t *error,
    uint32_t count, ...)
{
    va_list parameters;
    va_start(parameters, count);

    void *return_value = NULL;
    *error = (SHARED_OK);

    struct kore_pgsql database_connection;

    if(database_engine_initialize(&database_connection, DATABASE_NAME) != (SHARED_OK))
    {
        *error = (DATABASE_ENGINE_ERROR_INITIALIZATION);
        return_value = NULL;
        goto error_exit;
    }

    if(kore_pgsql_v_query_params(&database_connection, sql_query, 0, count, parameters) != 
        (KORE_RESULT_OK)) 
    {
        kore_pgsql_logerror(&database_connection);
        *error = (DATABASE_ENGINE_ERROR_QUERY_ERROR);
        return_value = NULL;
        goto error_exit; 
    }

    if(kore_pgsql_ntuples(&database_connection) == 0)
    {
        *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
        return_value = NULL;
        goto error_exit;
    }

    return_value = model_build_from_query(&database_connection, error);  

    error_exit:
    va_end(parameters);
    database_engine_close_connection(&database_connection);
    return return_value;
}

void 
database_engine_log_error(const char *prefix, uint32_t error)
{
    if(error == (SHARED_OK))
    {
        return;
    }

    switch(error)
    {
        case (DATABASE_ENGINE_ERROR_INITIALIZATION):
            kore_log(LOG_ERR, "%s: could not initialize database", prefix);
            break;
        case (DATABASE_ENGINE_ERROR_QUERY_ERROR):
            kore_log(LOG_ERR, "%s: query error", prefix);
            break;
        case (DATABASE_ENGINE_ERROR_RESULT_PARSE):
            kore_log(LOG_ERR, "%s: result parsing error", prefix);
            break;
        case (SHARED_ERROR_ALLOC_ERROR):
            kore_log(LOG_CRIT, "%s: failed allocation", prefix);
            break;
        default:
            kore_log(LOG_ERR, "%s: the database encountered a problem", prefix);
            break;
    }
}