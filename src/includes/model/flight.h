#ifndef FLIGHT_H
#define FLIGHT_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct
{
    uint32_t    flight_identifier;
    uint32_t    arrival_location_identifier;
    uint32_t    departure_location_identifier;
    char        *arrival_location;
    char        *departure_location; 
    struct tm   arrival_datetime;
    struct tm   departure_datetime;
    uint32_t    distance;
    uint32_t    seats_available;   
} Flight;

typedef struct FlightCollectionNode
{
    Flight *flight;
    TAILQ_ENTRY(FlightCollectionNode) flight_collection;
} FlightCollectionNode;

TAILQ_HEAD(FlightCollection, FlightCollectionNode); //expands to struct FlightCollection

Flight *
flight_create(
    uint32_t flight_identifier,
    char *departure_airport,
    char *arrival_airport,
    struct tm *departure_time,
    struct tm *arrival_time,
    uint32_t distance,
    uint32_t seats_available,
    uint32_t *error
    );

void *
flight_create_from_query(
    void *source_location,
    uint32_t *error
    );

void *
flight_collection_create_from_query(
    void *source_location,
    uint32_t *error
    );

void
flight_destroy(
    Flight **flight
    );

uint32_t
flight_insert(
    const Flight *flight
    );

uint32_t
flight_delete(
    Flight *flight
    );

uint32_t
flight_collection_destroy(
    struct FlightCollection *flight_collection
    );

Flight *
flight_find_by_identifier(
    uint32_t flight_identifier,
    uint32_t *error
    );

Flight *
flight_find_by_departure_airport(
    char * departure_airport,
    uint32_t *error
    );

Flight *
flight_find_by_arrival_airport(
    char *arrival_airport,
    uint32_t *error
    );

Flight *
flight_find_by_arrival_airport_and_departure_time(
    char *arrival_airport,
    struct tm *departure_time,
    uint32_t *error
    );

struct FlightCollection *
flight_find_by_arrival_date(
    struct tm *arrival_date, 
    uint32_t *error
    );

struct FlightCollection *
flight_get_all_flights(
    uint32_t *error
    );

#endif
