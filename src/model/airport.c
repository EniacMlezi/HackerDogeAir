#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <kore/kore.h>
#include <kore/pgsql.h>

#include "model/airport.h"
#include "shared/shared_time.h"
#include "shared/shared_error.h"
#include "model/database_engine.h"

static const char airrport_insert_query[] = 
    "INSERT INTO \"Airport\" (identifier, name, location) " \
    "VALUES ($1, $2, $3)";

static const char airrport_update_query[] = 
    "UPDATE \"Airport\" SET " \
    "identifier = $1 " \
    "name = $2 " \
    "location = $3 " \
    "WHERE identifier = $1;";

static const char airport_delete_query[] = "DELETE FROM \"Airport\" WHERE identifier = $1;";

static const char airport_find_by_identifier_query[] = 
    "SELECT identifier, name, location FROM \"Airport\" WHERE identifier = $1;";

static const char airport_find_by_name_query[] = 
    "SELECT identifier, name, location FROM \"Airport\" WHERE identifier = $1;";

static const char airport_find_by_location_query[] =
    "SELECT identifier, name, location FROM \"Airport\" WHERE location = $1;";

static const char airport_get_all_airports_query[] = 
    "SELECT identifier, name, location FROM \"Airport\";";

Airport *
airport_create(uint32_t identifier, const char *name, const char *location, uint32_t *error)
{
    uint8_t name_size = strlen(name);
    uint8_t location_size = strlen(location);    

    Airport *airport = malloc(sizeof(Airport) + name_size + location_size);

    if(airport == NULL)
    {
        kore_log(LOG_ERR, "airport_create: Could not allocate memory for the airport object.\n");
        *error = (SHARED_ERROR_ALLOC_ERROR);
        return NULL;
    }

    uint32_t memory_offset = sizeof(Airport);

    #pragma GCC diagnostic push  // require GCC 4.6
    #pragma GCC diagnostic ignored "-Wpointer-arith"

    airport->name = (void *) airport + memory_offset;
    memory_offset += name_size;

    airport->location = (void *) airport + memory_offset;
    memory_offset += location_size;

    #pragma GCC diagnostic pop   

    airport->identifier = identifier;
    strncpy(airport->name, name, name_size);
    strncpy(airport->location, location, location_size);

    return  airport;
}

void *
airport_create_from_query(void *source_location, uint32_t *error)
{
    if(kore_pgsql_nfields((struct kore_pgsql *) source_location) != 3)
    {
        kore_log(LOG_ERR, "airport_create_from_query: Invalid source location.");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    char *name = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 0);
    char *location = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 1);

    int err = 0;
    uint8_t identifier = kore_strtonum64(kore_pgsql_getvalue(
        (struct kore_pgsql *) source_location, 0, 2), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "airport_create_from_query: Could not translate db_identifier string to" \
            "uin32_t.");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t create_airport_result;
    Airport *temp_airport = airport_create(identifier, name, location, &create_airport_result);

    if(temp_airport == NULL)
    {
        kore_log(LOG_ERR, "airport_create_from_query: Could not create a airport structure.");
        *error = create_airport_result;
        return NULL;
    }

    *error = (SHARED_OK);
    return temp_airport;
}

void *
airport_collection_create_from_query(void *source_location, uint32_t *error)
{
    uint32_t number_of_result = kore_pgsql_ntuples((struct kore_pgsql *) source_location);

    struct AirportCollection *airport_collection = malloc(sizeof(struct AirportCollection));
    TAILQ_INIT(airport_collection);

    uint32_t i;
    for(i = 0; i < number_of_result; ++i)
    {
        char *name = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 0);
        char *location = kore_pgsql_getvalue((struct kore_pgsql *) source_location, i, 1);

        int err = 0;
        uint8_t identifier = kore_strtonum64(kore_pgsql_getvalue(
            (struct kore_pgsql *) source_location, i, 2), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "airport_create_from_query: Could not translate db_identifier " \
                " string to uin32_t.");
            *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
            airport_collection_destroy(&airport_collection);
            return NULL;
        }

        uint32_t create_airport_result;
        Airport *temp_airport = airport_create(identifier, name, location, &create_airport_result);

        if(temp_airport == NULL)
        {
            kore_log(LOG_ERR, "airport_create_from_query: Could not create a airport structure.");
            *error = create_airport_result;
            airport_collection_destroy(&airport_collection);
            return NULL;
        }       

        AirportCollectionNode *temp_airport_collection = malloc(sizeof(AirportCollectionNode));

        if(temp_airport_collection == NULL)
        {
            kore_log(LOG_ERR, "airport_collection_create_from_query: Could not allocate memory " \
                "for temp_airport_colection.");
            airport_destroy(&temp_airport);
            airport_collection_destroy(&airport_collection);
            *error = (SHARED_ERROR_ALLOC_ERROR);
            return NULL;
        } 

        temp_airport_collection->airport = temp_airport; 

        TAILQ_INSERT_TAIL(airport_collection, temp_airport_collection, airport_collection);

        temp_airport_collection = NULL;
    }

    *error = (SHARED_OK);
    return (void *) airport_collection;
}

void
airport_destroy(Airport **airport)
{
    if(airport == NULL)
    {
        return;
    }

    free(*airport);
    *airport = NULL;
}

uint32_t
airport_collection_destroy(struct AirportCollection **airport_collection)
{
    if(airport_collection == NULL || *airport_collection == NULL)
    {
        return (SHARED_OK);
    }

    AirportCollectionNode *temp = NULL;

    while(!TAILQ_EMPTY(*airport_collection))
    {
        temp = TAILQ_FIRST(*airport_collection);
        TAILQ_REMOVE(*airport_collection, temp, airport_collection);

        airport_destroy(&temp->airport);
        free(temp);
        temp = NULL;
    }

    free(*airport_collection);
    *airport_collection = NULL;

    return (SHARED_OK);
}

uint32_t
airport_insert(const Airport *airport)
{
    uint32_t identifier = htonl(airport->identifier);

    uint32_t query_result = database_engine_execute_write(airrport_insert_query, 3,
        &identifier, sizeof(identifier), 1,
        airport->name, strlen(airport->name), 0,
        airport->location, strlen(airport->location), 0);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("airport_insert", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

uint32_t
airport_update(const Airport *airport)
{
    uint32_t identifier = htonl(airport->identifier);

    uint32_t query_result = database_engine_execute_write(airrport_update_query, 3,
        &identifier, sizeof(identifier), 1,
        airport->name, strlen(airport->name), 0,
        airport->location, strlen(airport->location), 0);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("airport_update", query_result);
        return query_result;
    }

    return (SHARED_OK);   
}

uint32_t
airport_delete(Airport *airport)
{
    uint32_t identifier = htonl(airport->identifier);

    uint32_t query_result = database_engine_execute_write(airport_delete_query, 1,
        &identifier, sizeof(identifier), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("airport_delete", query_result);
        return query_result;
    }

    return (SHARED_OK);   
}

Airport *
airport_find_by_identifier(uint32_t identifier, uint32_t *error)
{
    uint32_t database_identifier = htonl(identifier);
    uint32_t query_result = 0; 

    void *result = database_engine_execute_read(airport_find_by_identifier_query, 
        &airport_create_from_query, &query_result, 1,
        &database_identifier, sizeof(database_identifier), 1);

    if(result == NULL)
    {
        database_engine_log_error("airport_find_by_identifier", query_result);
        *error = query_result;
    }

    return result;   
}

Airport *
airport_find_by_name(const char *name, uint32_t *error)
{
    uint32_t query_result = 0; 

    void *result = database_engine_execute_read(airport_find_by_name_query, 
        &airport_create_from_query, &query_result, 1,
        name, strlen(name), 0);

    if(result == NULL)
    {
        database_engine_log_error("airport_find_by_name", query_result);
        *error = query_result;
    }

    return result; 
}

Airport *
airport_find_by_location(const char *location, uint32_t *error)
{
    uint32_t query_result = 0; 

    void *result = database_engine_execute_read(airport_find_by_location_query, 
        &airport_create_from_query, &query_result, 1,
        location, strlen(location), 0);

    if(result == NULL)
    {
        database_engine_log_error("airport_find_by_location", query_result);
        *error = query_result;
    }

    return result; 
}

struct AirportCollection *
airport_get_all_airports(uint32_t *error)
{
    uint32_t query_result = 0; 

    void *result = database_engine_execute_read(airport_get_all_airports_query, 
        &airport_collection_create_from_query, &query_result, 0);

    if(result == NULL)
    {
        database_engine_log_error("airport_get_all_airports", query_result);
        *error = query_result;
    }

    return result; 
}