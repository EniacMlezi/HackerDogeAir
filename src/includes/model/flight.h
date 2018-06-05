#ifndef FLIGHT_H
#define FLIGHT_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct _Flight
{
    uint32_t    flight_identifier;
    uint32_t    departure_airport_identifier;
    uint32_t    arrival_airport_identifier;
    tm          departure_time;
    tm          arrival_time;
    double      distance;
    uint32_t    seats_available; 
} Flight;

typedef struct
{
    Flight flight;
    LIST_ENTRY(_Flight) list;
} FlightList;

Flight *
flight_create(
    uint32_t flight_identifier,
    uint32_t departure_airport_identifier,
    uint32_t arrival_airport_identifier,
    tm departure_time,
    tm arrival_time,
    double distance,
    uint32_t seats_available
    );

uint32_t
flight_destroy(
    Flight *flight
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
    uint32_t flight_identifier
    );

FlightList *
flight_list_create(
    void *source_location
    );

uint32_t
flight_list_destroy(
    FlightList *flight_list
    );

FlightList *
flight_list_find_by_path(
    uint32_t departure_airport_identifier,
    uint32_t arrival_airport_identifier
    );

#endif
