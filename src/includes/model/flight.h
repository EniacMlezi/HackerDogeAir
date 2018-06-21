#ifndef FLIGHT_H
#define FLIGHT_H

#include <time.h>

typedef struct Flight
{   //TODO: should move to model with introduction of DataAccess layer
    int id;
    time_t arrival_datetime;
    time_t departure_datetime;
    char *arrival_location;
    char *departure_location;
} Flight;

#endif