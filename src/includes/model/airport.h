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

typedef struct
{
    Airport airport; 
    LIST_ENTRY(_Airport) list;
} AirportList;

Airport *
airport_create(
    uint32_t identifier,
    char name[255],
    char location[255]
    );

uint32_t
airport_destroy(
    Airport *airport
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
    uint32_t identifier
    );

Airport *
airport_find_by_name(
    const char name[255]
    );

AirportList *
airport_list_create(
    void *source_location
    );

AirportList *
airport_get_list(
    void
    );

#endif