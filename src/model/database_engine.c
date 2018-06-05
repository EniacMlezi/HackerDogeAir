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

    return (DATABASE_ENGINE_OK);

    /* Error exit label. */
    error_exit: 
    kore_pgsql_cleanup(pgsql);
    return return_code;
}

static uint32_t
database_engine_close_connection(struct kore_pgsql *pgsql)
{
    kore_pgsql_cleanup(pgsql); 
    return (DATABASE_ENGINE_OK);
}

uint32_t
database_engine_execute_mutation_query(char *sql_query, 
    uint32_t count, va_list argument_list)
{
    uint32_t return_code = (DATABASE_ENGINE_OK);
    struct kore_pgsql database_connection;
    
    if(database_engine_initialize(&database_connection, DATABASE_NAME) != DATABASE_ENGINE_OK)
    {
        perror("database_engine_execute_mutation_query: Could not initialize the database.\n");
        return_code = (DATABASE_ENGINE_ERROR_INITIALIZATION);
        goto error_exit; 
    }    

    /* Execute the query on the connected database. */
    if(!kore_pgsql_query_params(&database_connection, sql_query, 0, count, argument_list))
    {
        kore_pgsql_logerror(&database_connection);
        return_code = (DATABASE_ENGINE_ERROR_QUERY_ERROR);
        goto error_exit;
    }

    error_exit:
    database_engine_close_connection(&database_connection);
    return return_code;
}

void *
database_engine_execute_select_query(char *sql_query, 
    void *(*model_build_from_query)(void *source_location), uint32_t count,
    va_list argument_list)
{
    void *return_value;
    struct kore_pgsql database_connection;

    if(database_engine_initialize(&database_connection, DATABASE_NAME) != DATABASE_ENGINE_OK)
    {
        perror("database_engine_execute_select_query: Could not initialize database.\n");
        return_value = (void *) (DATABASE_ENGINE_ERROR_INITIALIZATION);
        goto error_exit;
    }

    if(!kore_pgsql_query_params(&database_connection, sql_query, 0, count, argument_list)) 
    {
        kore_pgsql_logerror(&database_connection);
        return_value = (void *) (DATABASE_ENGINE_ERROR_QUERY_ERROR);
        goto error_exit; 
    }

    if(kore_pgsql_ntuples(&database_connection) == 0)
    {
        perror("database_engine_execute_select_query: No results found.");
        return_value = (void *) (DATABASE_ENGINE_ERROR_NO_RESULTS); 
        goto error_exit;
    }

    if((return_value = model_build_from_query(&database_connection))
     == (void *) (DATABASE_ENGINE_ERROR_RESULT_PARSE))
    {
        perror("database_engine_execute_select_query: Could not interpret the " \
            "query result.\n");
        return_value = (void *) (DATABASE_ENGINE_ERROR_INVALLID_RESULT);
    }
     
    error_exit:
    database_engine_close_connection(&database_connection);
    return return_value;
}