#ifndef TICKET_H
#define TICKET_H

#include <stdint.h>
#include <sys/queue.h>

typedef struct 
{
    uint32_t    ticket_identifier;
    uint32_t    flight_identifier;
    uint32_t    user_identifier;
    double      cost;
} Ticket;

typedef struct TicketCollection
{
    Ticket ticket;
    LIST_ENTRY(TicketCollection) ticket_collection;
} TicketCollection;

Ticket *
ticket_create(
    uint32_t ticket_identifier,
    uint32_t flight_identifier,
    uint32_t user_identifier,
    double cost,
    uint32_t *error
    );

uint32_t
ticket_destroy(
    Ticket *ticket
    );

uint32_t
ticket_insert(
    const Ticket *ticket
    );

uint32_t
ticket_update(
    const Ticket *ticket
    );

uint32_t
ticket_delete(
    Ticket *ticket
    );

Ticket *
ticket_find_by_user_identifier(
    uint32_t user_identifier,
    uint32_t *error
    );

TicketCollection *
ticket_collection_create(
    void *source_location,
    uint32_t *error
    );

uint32_t
ticket_collection_destroy(
    TicketCollection *ticket_collection
    );

TicketCollection *
ticket_collection_find_by_flight_identifier(
    uint32_t flight_identifier,
    uint32_t *error
    );

TicketCollection *
ticket_collection_find_by_user_identifier(
    uint32_t user_identifier,
    uint32_t *error
    );

#endif