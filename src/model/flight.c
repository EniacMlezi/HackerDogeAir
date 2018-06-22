#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <kore/kore.h>
#include <kore/pgsql.h>

#include "model/flight.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"
#include "model/database_engine.h"

static const char flight_find_by_arrival_date_query[] =
"SELECT flightidentifier,a.name,d.name,arrivaldatetime,departuredatetime,distance,seatsavailable" \
"FROM \"Flight\" INNER JOIN \"Airport\" AS a ON a.airportidentifier = arrivalairportidentifier" \
"INNER JOIN \"Airport\" AS d ON d.airportidentifier = departureairportidentifier"\
"WHERE flightidentifier=$1";

static const char flight_insert_query[] =
"INSERT INTO \"Flight\"";

Flight *
flight_create(uint32_t identifier, char *departure_location, char *arrival_location,
    struct tm *departure_time, struct tm *arrival_time, uint32_t distance, uint32_t seats_available,
    uint32_t *error)
{
    /* Allocating space for the structure and the strings appended to it. */
    uint8_t departure_location_size = strlen(departure_location);
    uint8_t arrival_location_size = strlen(arrival_location);
    Flight *flight = malloc(sizeof(Flight) + departure_location_size + arrival_location_size); 
    if(flight == NULL)
    {
        perror("user_create: Could not allocate memory for the user object.\n");
        *error = (SHARED_ERROR_ALLOC_ERROR);
        return NULL;
    }

    uint32_t memory_offset = sizeof(Flight);

    /* Initializing the flight structure. */
    flight->departure_location = (void *)flight + memory_offset;
    memory_offset += departure_location_size;
    flight->arrival_location = (void *)flight + memory_offset;
    memory_offset += arrival_location_size;

    flight->identifier = identifier;
    strncpy(flight->departure_location, departure_location, departure_location_size);
    strncpy(flight->arrival_location, arrival_location, arrival_location_size);
    flight->departure_datetime = *departure_time;
    flight->arrival_datetime = *arrival_time;
    flight->distance = distance;
    flight->seats_available = seats_available;

    return flight;
}

void
flight_destroy(Flight *flight)
{
    free(flight);
}

void *
flight_create_from_query(void *source_location, uint32_t *error)
{
    struct kore_pgsql *pgsql = (struct kore_pgsql *)source_location;

    if(kore_pgsql_nfields(pgsql) != 7)
    {
        kore_log(LOG_ERR, "flight_create_from_query: Invalid source location");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    char *arrival_location = kore_pgsql_getvalue(pgsql, 0, 1);
    char *departure_location = kore_pgsql_getvalue(pgsql, 0, 2);
    char *arrival_time_string = kore_pgsql_getvalue(pgsql, 0, 3);
    char *departure_time_string = kore_pgsql_getvalue(pgsql, 0, 4);

    struct tm arrival_time;
    struct tm departure_time;
    shared_time_database_string_to_tm(arrival_time_string, &arrival_time);
    shared_time_database_string_to_tm(departure_time_string, &departure_time);

    int err = 0;
    uint32_t identifier = kore_strtonum64(kore_pgsql_getvalue(pgsql, 0, 0), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_flight_id string to " \
            "uint32_t.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }
    uint32_t distance = kore_strtonum64(kore_pgsql_getvalue(pgsql, 0, 5), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_distance string to " \
            "uint32_t.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }
    uint32_t seats_available = kore_strtonum64(kore_pgsql_getvalue(pgsql, 0, 6), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_seatsavailable string" \
            " to uint32_t.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t create_flight_result;
    Flight *flight = flight_create(identifier, departure_location, arrival_location, &departure_time,
        &arrival_time, distance, seats_available, &create_flight_result);

    if(flight == NULL)
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not create a flight structure");
    }
}

void *
flight_collection_create_from_query(void *source_location, uint32_t *error)
{
    struct kore_pgsql *pgsql = (struct kore_pgsql *) source_location;

}

uint32_t
flight_insert(const Flight *flight)
{
    uint32_t error;
    uint32_t distance = htonl(flight->distance);
    uint32_t seats_available = htonl(flight->seats_available);
    uint32_t arrival_location = htonl(flight->arrival_location_identifier);
    uint32_t departure_location = htonl(flight->arrival_location_identifier);

    char arrival_datetime[30]; //TODO: find appropriate size
    shared_time_tm_to_database_string(&flight->arrival_datetime, arrival_datetime, 
        sizeof(arrival_datetime), &error);
    if(error != (SHARED_OK))
    {
        kore_log(LOG_ERR, "flight_insert: Time conversion error: %d", error);
        return (SHARED_ERROR_TIME_CONVERSION);
    }

    char departure_datetime[30]; //TODO: find appropriate size
    shared_time_tm_to_database_string(&flight->departure_datetime, departure_datetime, 
        sizeof(departure_datetime), &error);
    if(error != (SHARED_OK))
    {
        kore_log(LOG_ERR, "flight_insert: Time conversion error: %d", error);
        return (SHARED_ERROR_TIME_CONVERSION);
    }

    uint32_t query_result = database_engine_execute_write(flight_insert_query, 7,
        &arrival_location, sizeof(arrival_location), 1,
        &departure_location, sizeof(departure_location), 1,
        arrival_datetime, strlen(arrival_datetime), 0,
        departure_datetime, strlen(departure_datetime), 0,
        &distance, sizeof(distance), 1,
        &seats_available, sizeof(seats_available), 1
        );

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("flight_insert", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

FlightCollection *
flight_find_by_arrival_date(struct tm *arrival_date, uint32_t *error)
{
    uint32_t err = 0;
    uint32_t query_result;
    void *result;

    char arrival_datetime[30]; //TODO: find appropriate size
    shared_time_tm_to_database_string(arrival_date, arrival_datetime, 
        sizeof(arrival_datetime), &err);
    if(err != (SHARED_OK))
    {
        kore_log(LOG_ERR, "flight_insert: Time conversion error: %d", err);
        *error = (SHARED_ERROR_TIME_CONVERSION); 
        return NULL;
    }

    result = database_engine_execute_read(flight_find_by_arrival_date_query, 
        &flight_collection_create_from_query, &err, 1, 
        &arrival_datetime, strlen(arrival_datetime), 0);
}