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
"SELECT flightidentifier,a.name,d.name,arrivaldatetime,departuredatetime,distance,seatsavailable " \
"FROM \"Flight\" INNER JOIN \"Airport\" AS a ON a.airportidentifier = arrivalairportidentifier " \
"INNER JOIN \"Airport\" AS d ON d.airportidentifier = departureairportidentifier "\
"WHERE arrivaldatetime::date = $1;";

static const char flight_insert_query[] =
    "INSERT INTO \"Flight\" " \
    "flightidentifier,departureairportidentifier,arrivalairportidentifier,departuredatetime," \
    "arrivaldatetime,distance,seatsavailabe) " \
    "VALUES ($1, $2, $3, $4, $5, $6, $7);";

static const char flight_delete_query[] =
    "DELETE FROM \"Flight\" WHERE flightidentifier = $1;";

static const char flight_get_all_flights_query[] = 
"SELECT flightidentifier,a.name,d.name,arrivaldatetime,departuredatetime,distance,seatsavailable" \
"FROM \"Flight\" INNER JOIN \"Airport\" AS a ON a.airportidentifier = arrivalairportidentifier" \
"INNER JOIN \"Airport\" AS d ON d.airportidentifier = departureairportidentifier;";

static const char flight_find_by_identifier_query[] = 
"SELECT flightidentifier,a.name,d.name,arrivaldatetime,departuredatetime,distance,seatsavailable" \
"FROM \"Flight\" INNER JOIN \"Airport\" AS a ON a.airportidentifier = arrivalairportidentifier" \
"INNER JOIN \"Airport\" AS d ON d.airportidentifier = departureairportidentifier"\
"WHERE flightidentifier=$1;";

static const char flight_find_by_departure_airport_query[] = 
"SELECT flightidentifier,a.name,d.name,arrivaldatetime,departuredatetime,distance,seatsavailable" \
"FROM \"Flight\" INNER JOIN \"Airport\" AS a ON a.airportidentifier = arrivalairportidentifier" \
"INNER JOIN \"Airport\" AS d ON d.airportidentifier = departureairportidentifier"\
"WHERE d.name=$1;";

static const char flight_find_by_arrival_airport_query[] = 
"SELECT flightidentifier,a.name,d.name,arrivaldatetime,departuredatetime,distance,seatsavailable" \
"FROM \"Flight\" INNER JOIN \"Airport\" AS a ON a.airportidentifier = arrivalairportidentifier" \
"INNER JOIN \"Airport\" AS d ON d.airportidentifier = departureairportidentifier"\
"WHERE a.name=$1;";

static const char flight_find_by_arrival_airport_and_departure_time_query[] =     
"SELECT flightidentifier,a.name,d.name,arrivaldatetime,departuredatetime,distance,seatsavailable" \
"FROM \"Flight\" INNER JOIN \"Airport\" AS a ON a.airportidentifier = arrivalairportidentifier" \
"INNER JOIN \"Airport\" AS d ON d.airportidentifier = departureairportidentifier"\
"WHERE a.name=$1 AND departuredatetime = $2;";

Flight *
flight_create(uint32_t flight_identifier, char *departure_location, char *arrival_location,
    struct tm *departure_time, struct tm *arrival_time, uint32_t distance, uint32_t seats_available,
    uint32_t *error)
{
    /* Allocating space for the structure and the strings appended to it. */
    uint8_t departure_location_size = strlen(departure_location) + 1;
    uint8_t arrival_location_size = strlen(arrival_location) + 1;

    Flight *flight = malloc(sizeof(Flight) + departure_location_size + arrival_location_size); 

    if(flight == NULL)
    {
        kore_log(LOG_ERR, "user_create: Could not allocate memory for the user object.\n");
        *error = (SHARED_ERROR_ALLOC_ERROR);
        return NULL;
    }

    uint32_t memory_offset = sizeof(Flight);

    #pragma GCC diagnostic push  // require GCC 4.6
    #pragma GCC diagnostic ignored "-Wpointer-arith"

    /* Initializing the flight structure. */
    flight->departure_location = (void *) flight + memory_offset;
    memory_offset += departure_location_size;
    
    flight->arrival_location = (void *) flight + memory_offset;
    memory_offset += arrival_location_size;

    #pragma GCC diagnostic pop

    flight->flight_identifier = flight_identifier;
    strncpy(flight->departure_location, departure_location, departure_location_size);
    strncpy(flight->arrival_location, arrival_location, arrival_location_size);
    flight->departure_datetime = *departure_time;
    flight->arrival_datetime = *arrival_time;
    flight->distance = distance;
    flight->seats_available = seats_available;

    return flight;
}

void *
flight_create_from_query(void *source_location, uint32_t *error)
{
    if(kore_pgsql_nfields((struct kore_pgsql *) source_location) != 7)
    {
        kore_log(LOG_ERR, "flight_create_from_query: Invalid source location.");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    char *arrival_location = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 1);
    char *departure_location = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 2);
    char *arrival_time_string = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 3);
    char *departure_time_string = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 4);

    struct tm arrival_time;
    struct tm departure_time;
    shared_time_database_string_to_tm(arrival_time_string, &arrival_time);
    shared_time_database_string_to_tm(departure_time_string, &departure_time);

    int err = 0;
    uint32_t flight_identifier = kore_strtonum64(kore_pgsql_getvalue(
        (struct kore_pgsql *) source_location, 0, 0), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_flight_id string to " \
            "uint32_t.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t distance = kore_strtonum64(kore_pgsql_getvalue((struct kore_pgsql *) source_location,
        0, 5), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_distance string to " \
            "uint32_t.");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t seats_available = kore_strtonum64(kore_pgsql_getvalue(
        (struct kore_pgsql *) source_location, 0, 6), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_seatsavailable string" \
            " to uint32_t.");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    uint32_t create_flight_result;
    Flight *temp_flight = flight_create(flight_identifier, departure_location, arrival_location, 
        &departure_time, &arrival_time, distance, seats_available, &create_flight_result);

    if(temp_flight == NULL)
    {
        kore_log(LOG_ERR, "flight_create_from_query: Could not create a flight structure.");
        *error = create_flight_result;
        return NULL;
    }

    *error = (SHARED_OK);
    return temp_flight;
}

void *
flight_collection_create_from_query(void *source_location, uint32_t *error)
{
    uint32_t number_of_results = kore_pgsql_ntuples((struct kore_pgsql *) source_location);

    struct FlightCollection *flightcollection = malloc(sizeof(struct FlightCollection));
    TAILQ_INIT(flightcollection);

    uint32_t i;
    for(i = 0; i < number_of_results; ++i)
    {
        Flight *temp_flight = NULL;

        char *arrival_location = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 1);
        char *arrival_time_string = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 2);
        char *departure_location = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 3);
        char *departure_time_string = kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 4);

        struct tm arrival_time;
        struct tm departure_time;
        shared_time_database_string_to_tm(arrival_time_string, &arrival_time);
        shared_time_database_string_to_tm(departure_time_string, &departure_time);

        int err = 0;
        uint32_t flight_identifier = kore_strtonum64(kore_pgsql_getvalue(
            (struct kore_pgsql *) source_location, i, 0), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_flight_id string to " \
                "uint32_t.");
            *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
            free(flightcollection);
            return NULL;
        }

        uint32_t distance = kore_strtonum64(kore_pgsql_getvalue(
            (struct kore_pgsql *) source_location, i, 5), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_distance string to " \
                "uint32_t.");
            *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
            free(flightcollection);
            return NULL;
        }

        uint32_t seats_available = kore_strtonum64(kore_pgsql_getvalue(
            (struct kore_pgsql *) source_location, i, 6), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "flight_create_from_query: Could not translate db_seatsavailable string" \
                " to uint32_t.");
            *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
            free(flightcollection);
            return NULL;
        }

        uint32_t create_flight_result;
        temp_flight = flight_create(flight_identifier, departure_location, arrival_location, 
            &departure_time, &arrival_time, distance, seats_available, &create_flight_result);

        if(temp_flight == NULL)
        {
            kore_log(LOG_ERR, "flight_create_from_query: Could not create a flight structure");
            *error = create_flight_result;
            free(flightcollection);
            return NULL;
        }        

        FlightCollectionNode *temp_flight_node = malloc(sizeof(FlightCollectionNode));

        if(temp_flight_node == NULL)
        {
            kore_log(LOG_ERR, "flight_create_from_query: Could not allocate memory for " \
                "temp_flight_collection.");
            flight_destroy(&temp_flight);
            //flight_collection_destroy(flight_collection);
            free(flightcollection);
            return NULL;   
        }

        temp_flight_node->flight = temp_flight;

        TAILQ_INSERT_TAIL(flightcollection, temp_flight_node, flight_collection);
        
        temp_flight_node = NULL;
    }

    return (void *) flightcollection;
}

void
flight_destroy(Flight **flight)
{
    free(*flight);
    *flight = NULL;
}

uint32_t
flight_insert(const Flight *flight)
{
    uint32_t error = 0;
    uint32_t distance = htonl(flight->distance);
    uint32_t seats_available = htonl(flight->seats_available);
    uint32_t arrival_location = htonl(flight->arrival_location_identifier);
    uint32_t departure_location = htonl(flight->arrival_location_identifier);

    char arrival_datetime[SHARED_TIME_TM_TO_DATABASE_FORMAT_STRING_SIZE];
    shared_time_tm_to_database_string(&flight->arrival_datetime, arrival_datetime, 
        sizeof(arrival_datetime), &error);
    if(error != (SHARED_OK))
    {
        kore_log(LOG_ERR, "flight_insert: Time conversion error: %d", error);
        return (SHARED_ERROR_TIME_CONVERSION);
    }

    char departure_datetime[SHARED_TIME_TM_TO_DATABASE_FORMAT_STRING_SIZE];
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

uint32_t
flight_delete(Flight *flight)
{
    uint32_t flight_identifier = htonl(flight->flight_identifier);

    uint32_t query_result = database_engine_execute_write(flight_delete_query, 1, 
        &flight_identifier, sizeof(flight_identifier), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("flight_delete", query_result);
        return query_result;
    }

    return (SHARED_OK);  
}

uint32_t
flight_collection_destroy(struct FlightCollection *flight_collection)
{
    FlightCollectionNode *temp = NULL;
    while(!TAILQ_EMPTY(flight_collection))
    {
        temp = TAILQ_FIRST(flight_collection);
        TAILQ_REMOVE(flight_collection, temp, flight_collection);

        flight_destroy(&temp->flight);
        free(temp);
        temp = NULL;
    }

    free(flight_collection);
    flight_collection = NULL;

    return (SHARED_OK);
}


Flight *
flight_find_by_identifier(uint32_t flight_identifier, uint32_t *error)
{
    uint32_t query_result;
    void *result;

    uint32_t database_flight_identifier = htonl(flight_identifier);

    result = database_engine_execute_read(flight_find_by_identifier_query, 
        &flight_create_from_query, &query_result, 1, 
        &database_flight_identifier, sizeof(database_flight_identifier), 1);

    if(result == NULL)
    {
        database_engine_log_error("flight_find_by_identifier", query_result);
        *error = query_result;
    }

    return result;
}

Flight *
flight_find_by_departure_airport(char *departure_airport, uint32_t *error)
{
    uint32_t query_result;
    void *result;

    result = database_engine_execute_read(flight_find_by_departure_airport_query, 
        &flight_create_from_query, &query_result, 1, 
        &departure_airport, strlen(departure_airport), 0);

    if(result == NULL)
    {
        database_engine_log_error("flight_find_by_departure_airport", query_result);
        *error = query_result;
    }

    return result;
}

Flight *
flight_find_by_arrival_airport(char *arrival_airport, uint32_t *error)
{
    uint32_t query_result;
    void *result;


    result = database_engine_execute_read(flight_find_by_arrival_airport_query, 
        &flight_create_from_query, &query_result, 1, 
        &arrival_airport, strlen(arrival_airport), 0);

    if(result == NULL)
    {
        database_engine_log_error("flight_find_by_arrival_airport", query_result);
        *error = query_result;
    }

    return result;
}

Flight *
flight_find_by_arrival_airport_and_departure_time(char *arrival_airport,
    struct tm *departure_time, uint32_t *error)
{
    uint32_t query_result = 0;
    void *result;

    uint32_t err = 0;

    char database_departure_time[SHARED_TIME_TM_TO_DATABASE_FORMAT_STRING_SIZE];

    shared_time_tm_to_database_string(departure_time, database_departure_time, 
        SHARED_TIME_TM_TO_DATABASE_FORMAT_STRING_SIZE, &err);

    if(err != (SHARED_OK))
    {
        kore_log(LOG_ERR, "flight_find_by_arrival_airport_and_departure_time: struct tm to " \
            "string conversion error detected: %d.", err);
        *error = (SHARED_ERROR_TIME_CONVERSION);
        return NULL;
    }

    result = database_engine_execute_read(flight_find_by_arrival_airport_and_departure_time_query, 
        &flight_create_from_query, &query_result, 2, 
        arrival_airport, strlen(arrival_airport), 0,
        database_departure_time, strlen(database_departure_time), 0);

    if(result == NULL)
    {
        database_engine_log_error("flight_find_by_arrival_airport_and_departure_time", 
            query_result);
        *error = query_result;
    }

    return result;
}


struct FlightCollection *
flight_find_by_arrival_date(struct tm *arrival_date, uint32_t *error)
{
    uint32_t err = 0;
    uint32_t query_result = 0;
    void *result;

    char arrival_datetime[30];

    if((err = shared_time_tm_to_string(arrival_date, arrival_datetime, 
        sizeof(arrival_datetime), "%Y-%m-%d")) != (SHARED_OK))
    {
        kore_log(LOG_ERR, "flight_find_by_arrival_date: struct tm to string conversion error " \
            "detected: %d.", err);
        *error = (SHARED_ERROR_TIME_CONVERSION); 
        return NULL;
    }

    result = database_engine_execute_read(flight_find_by_arrival_date_query, 
        &flight_collection_create_from_query, &err, 1, 
        &arrival_datetime, strlen(arrival_datetime), 0);
   
    if(result == NULL)
    {
        database_engine_log_error("flight_find_by_arrival_date", query_result);
        *error = query_result;
    }

    return result;
}

struct FlightCollection *
flight_get_all_flights(uint32_t *error)
{
    uint32_t query_result;
    void *result;

    result = database_engine_execute_read(flight_get_all_flights_query, 
        &flight_collection_create_from_query, &query_result, 0); 

    if(result == NULL)
    {
        database_engine_log_error("flight_get_all_flights", query_result);
        *error = query_result;
    }

    return result;
}