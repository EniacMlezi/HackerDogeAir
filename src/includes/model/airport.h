#ifndef AIRPORT_H
#define AIRPORT_H

#include <stdint.h>
#include <sys/queue.h>

typedef struct _Airport
{
    uint32_t    identifier;
    char        *name;
    char        location[255];   
} Airport; 

typedef struct AirportCollection
{
    Airport airport; 
    LIST_ENTRY(AirportCollection) airport_collection;
} AirportCollection;

Airport *
airport_create(
    uint32_t identifier,
    char name[255],
    char location[255],
    uint32_t *error
    );

void *
airport_create_from_query(
    void *source_location,
    uint32_t *error
    );

void *
airport_collection_create_from_query(
    void *source_location,
    uint32_t *error
    );

uint32_t
airport_destroy(
    Airport **airport
    );

uint32_t
airport_collection_destroy(
    AirportCollection *airport_collection
    );

uint32_t
airport_insert(
    const Airport *airport
    );

uint32_t
airport_update(
    const Airport *airport
    );

uint32_t
airport_delete(
    Airport *airport
    );

Airport *
airport_find_by_identifier(
    uint32_t identifier,
    uint32_t *error
    );

Airport *
airport_find_by_name(
    const char name[255],
    uint32_t *error
    );

AirportCollection *
airport_get_collection(
    uint32_t *error
    );

#endif