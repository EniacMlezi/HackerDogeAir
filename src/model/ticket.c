#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>

#include <kore/kore.h>
#include <kore/pgsql.h>
#include <kore/http.h>

#include "includes/model/ticket.h"
#include "includes/model/database_engine.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"

static const char ticket_insert_query[] = "";

static const char ticket_select_by_ticket_identifier[] = "";

static const char ticket_select_by_flight_identifier[] = "";

static const char ticket_select_by_user_identifer[] = "";

static const char ticket_update_query[] = "";

static const char ticket_delete_query[] = "";

Ticket *
ticket_create(uint32_t ticket_identifier, uint32_t flight_identifier, uint32_t user_identifier,
    double cost, uint32_t *error)
{

}

uint32_t
ticket_destroy(Ticket *ticket)
{

}

uint32_t
ticket_insert(const Ticket *ticket)
{

}

uint32_t
ticket_update(const Ticket *ticket)
{

}

uint32_t
ticket_delete(Ticket *ticket)
{

}

Ticket *
ticket_find_by_user_identifier(uint32_t user_identifier, uint32_t *error)
{

}

TicketCollection *
ticket_collection_create(void *source_location, uint32_t *error)
{

}

uint32_t
ticket_collection_destroy(TicketCollection *ticket_collection)
{

}

TicketCollection *
ticket_collection_find_by_flight_identifier(uint32_t flight_identifier, uint32_t *error)
{

}

TicketCollection *
ticket_collection_find_by_user_identifier(uint32_t user_identifier, uint32_t *error)
{

}