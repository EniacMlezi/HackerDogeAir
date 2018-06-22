#ifndef FLIGHT_H
#define FLIGHT_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct
{
    uint32_t    flight_identifier;
    uint32_t    departure_airport_identifier;
    uint32_t    arrival_airport_identifier;
    char        *departure_time;
    char        *arrival_time;
    double      distance;
    uint32_t    seats_available; 
    struct tm   arrival_datetime;
    struct tm   departure_datetime;
    char        *arrival_location;
    char        *departure_location;   
} Flight;

typedef struct FlightCollection
{
    Flight flight;
    TAILQ_ENTRY(_Flight) flight_collection;
} FlightCollection;

Flight *
flight_create(
    uint32_t flight_identifier,
    uint32_t departure_airport_identifier,
    uint32_t arrival_airport_identifier,
    char *departure_time,
    char *arrival_time,
    double distance,
    uint32_t seats_available,
    uint32_t *error
    );

uint32_t
flight_destroy(
    Flight **flight
    );

uint32_t
flight_insert(
    const Flight *flight
    );

uint32_t
flight_update(
    const Flight *flight
    );

uint32_t
flight_delete(
    Flight *flight
    );

Flight *
flight_find_by_identifier(
    uint32_t flight_identifier,
    uint32_t *error
    );

Flight *
flight_find_by_departure_airport(
    uint32_t departure_airport_identifier,
    uint32_t *error
    );

Flight *
flight_find_by_arrival_airport(
    uint32_t arrival_airport_identifier,
    uint32_t *error
    );

uint32_t
flight_delete_by_flight_identifier(
    uint32_t flight_identifier
    );

Flight *
flight_find_by_arrival_airport_and_departure_time(
    uint32_t arrival_airport_identifier,
    const char *departure_time,
    uint32_t *error
    );

FlightCollection *
flight_collection_create(
    void *source_location,
    uint32_t *error
    );

uint32_t
flight_collection_destroy(
    FlightCollection *flight_collection
    );

FlightCollection *
flight_collection_find_by_path(
    uint32_t departure_airport_identifier,
    uint32_t arrival_airport_identifier,
    uint32_t *error
    );

#endif
