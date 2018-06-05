#ifndef TICKET_H
#define TICKET_H

#include <stdint.h>
#include <sys/queue.h>

typedef struct _Ticket
{
    uint32_t    ticket_identifier;
    uint32_t    flight_identifier;
    uint32_t    user_identifier;
    double      cost;
} Ticket;

typedef struct
{
    Ticket ticket;
    LIST_ENTRY(_Ticket) list;
} TicketList;

Ticket *
ticket_create(
    uint32_t ticket_identifier,
    uint32_t flight_identifier,
    uint32_t user_identifier,
    double cost
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
    uint32_t user_identifier
    );

TicketList *
ticket_list_create(
    void *source_location
    );

uint32_t
ticket_list_destroy(
    TicketList *ticket_list
    );

TicketList *
ticket_list_find_by_flight_identifier(
    uint32_t flight_identifier
    );

TicketList *
ticket_list_find_by_user_identifier(
    uint32_t user_identifier
    );

#endif