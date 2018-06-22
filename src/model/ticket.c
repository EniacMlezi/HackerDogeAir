#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>

#include <kore/kore.h>
#include <kore/pgsql.h>
#include <kore/http.h>

#include "model/ticket.h"
#include "model/database_engine.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"

static const char ticket_insert_query[] = 
    "INSERT INTO \"Ticket\" (ticketidentifier, flightidentifier useridentifier, cost) " \
    "VALUES ($1, $2, $3, $4)";

static const char ticket_select_by_ticket_identifier[] = 
    "SELECT ticketidentifier, flightidentifier, useridentifer, cost FROM \"Ticket\" " \
    "WHERE ticketidentifier = $1;";

static const char ticket_select_by_flight_identifier[] =
    "SELECT ticketidentifier, flightidentifier, useridentifer, cost FROM \"Ticket\" " \
    "WHERE flightidentifier = $1;";

static const char ticket_select_by_user_identifier[] =
    "SELECT ticketidentifier, flightidentifier, useridentifer, cost FROM \"Ticket\" " \
    "WHERE useridentifer = $1;";

static const char ticket_update_query[] = 
    "UPDATE \"Ticket\" SET ticketidentifier = $1, flightidentifier = $2, useridentifer = $3, " \
    "cost = $4 WHERE ticketidentifier = $1";

static const char ticket_delete_query[] =
    "DELETE FROM \"Session\" WHERE sessionidentifier = $1";

Ticket *
ticket_create(uint32_t ticket_identifier, uint32_t flight_identifier, uint32_t user_identifier,
    uint32_t cost, uint32_t *error)
{
    Ticket *ticket = malloc(sizeof(Ticket));

    if(ticket == NULL)
    {
        kore_log(LOG_ERR, "ticket_create: Could not allocate memory for a ticket structure.\n");
        *error = (SESSION_ERROR_CREATE);
        return NULL;
    }

    ticket->ticket_identifier = ticket_identifier;
    ticket->flight_identifier = flight_identifier;
    ticket->user_identifier = user_identifier;
    ticket->cost = cost;

    *error = (SHARED_OK);
    return ticket;
}

void
ticket_destroy(Ticket *ticket)
{
    free(ticket);
    ticket = NULL;
}

void *
ticket_create_from_query(void *source_location, uint32_t *error)
{
    if(kore_pgsql_nfields((struct kore_pgsql *) source_location) != 10)
    {
        perror("ticket_create_from_query: Invalid source location.\n");
        *error = (DATABASE_ENGINE_ERROR_RESULT_PARSE);
        return NULL;
    }

    int err = 0;
    uint32_t ticket_identifier = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 0), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_ticket_identifier " \
            "string to uint32_t.");
        *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
        return NULL;
    }

    uint32_t flight_identifier = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 1), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_flight_identifier " \
            "string to uint32_t.");
        *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
        return NULL;
    }

    uint32_t user_identifier = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 2), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_user_identifier " \
            "string to uint32_t.");
        *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
        return NULL;
    }

    uint32_t cost = kore_strtonum64(
        kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 3), 0, &err);
    if(err != (KORE_RESULT_OK))
    {
        kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_cost " \
            "string to uint32_t.");
        *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
        return NULL;
    }

    uint32_t create_ticket_result = 0;
    void *temp_ticket = ticket_create(ticket_identifier, flight_identifier, user_identifier, cost,
        &create_ticket_result);

    if(temp_ticket == NULL)
    {
        kore_log(LOG_ERR, "ticket_create_from_query: Could not create a ticket structure.");
        *error = create_ticket_result;
    }

    return temp_ticket;
}

void *
ticket_create_collection_from_query(void *source_location, uint32_t *error)
{
    uint32_t number_of_results = kore_pgsql_ntuples((struct kore_pgsql *) source_location);

    TAILQ_HEAD(ticket_collection_s, TicketCollection) *ticket_collection = 
        malloc(sizeof(TicketCollection)); 
    TAILQ_INIT(ticket_collection);

    uint32_t i;
    for(i = 0; i < number_of_results; ++i)
    {
        Ticket *ticket = NULL;

        int err = 0;
        uint32_t ticket_identifier = kore_strtonum64(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 0), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_ticket_identifier " \
                "string to uint32_t.");
            *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
            return NULL;
        }

        uint32_t flight_identifier = kore_strtonum64(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 1), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_flight_identifier " \
                "string to uint32_t.");
            *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
            return NULL;
        }

        uint32_t user_identifier = kore_strtonum64(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 2), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_user_identifier " \
                "string to uint32_t.");
            *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
            return NULL;
        }

        uint32_t cost = kore_strtonum64(
            kore_pgsql_getvalue((struct kore_pgsql *) source_location, 0, 3), 0, &err);
        if(err != (KORE_RESULT_OK))
        {
            kore_log(LOG_ERR, "ticket_create_from_query: Could not translate db_cost " \
                "string to uint32_t.");
            *error = (DATABASE_ENGINE_ERROR_NO_RESULTS);
            return NULL;
        }

        uint32_t create_ticket_result = 0;
        void *temp_ticket = ticket_create(ticket_identifier, flight_identifier, user_identifier, cost,
            &create_ticket_result);

        if(temp_ticket == NULL)
        {
            kore_log(LOG_ERR, "ticket_create_from_query: Could not create a ticket structure.");
            *error = create_ticket_result;
            return NULL;
        } 

        TicketCollection *temp_ticket_collection = malloc(sizeof(TicketCollection));
        temp_ticket_collection = temp_ticket;

        TAILQ_INSERT_TAIL(ticket_collection, temp_ticket_collection, ticket_collection);
        temp_ticket_collection = NULL;
    }

    return (void *) ticket_collection;
}

uint32_t
ticket_collection_destroy(TicketCollection *ticket_collection)
{

}

uint32_t
ticket_insert(const Ticket *ticket)
{
    uint32_t ticket_identifier = htonl(ticket->ticket_identifier);
    uint32_t flight_identifier = htonl(ticket->flight_identifier);
    uint32_t user_identifier   = htonl(ticket->user_identifier);
    uint32_t cost = htonl(ticket->cost);

    uint32_t query_result = database_engine_execute_write(ticket_insert_query, 4,
        &ticket_identifier, sizeof(ticket_identifier), 1,
        &flight_identifier, sizeof(flight_identifier), 1,
        &user_identifier, sizeof(user_identifier), 1,
        &cost, sizeof(cost), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("ticket_insert", query_result);
        return query_result;
    }

    return (SHARED_OK);
}

uint32_t
ticket_update(const Ticket *ticket)
{
    uint32_t ticket_identifier = htonl(ticket->ticket_identifier);
    uint32_t flight_identifier = htonl(ticket->flight_identifier);
    uint32_t user_identifier   = htonl(ticket->user_identifier);
    uint32_t cost = htonl(ticket->cost);

    uint32_t query_result = database_engine_execute_write(ticket_update_query, 4,
        &ticket_identifier, sizeof(ticket_identifier), 1,
        &flight_identifier, sizeof(flight_identifier), 1,
        &user_identifier, sizeof(user_identifier), 1,
        &cost, sizeof(cost), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("ticket_update", query_result);
        return query_result;
    }

    return (SHARED_OK);  
}

uint32_t
ticket_delete(Ticket *ticket)
{
    uint32_t ticket_identifier = htonl(ticket->ticket_identifier);

    uint32_t query_result = database_engine_execute_write(ticket_delete_query, 1,
        &ticket_identifier, sizeof(ticket_identifier), 1);

    if(query_result != (SHARED_OK))
    {
        database_engine_log_error("ticket_delete", query_result);
        return query_result;
    }

    return (SHARED_OK);  
}

Ticket *
ticket_find_by_user_identifier(uint32_t user_identifier, uint32_t *error)
{
    uint32_t database_user_identifier = htonl(user_identifier);

    void *result;
    uint32_t query_result = 0;

    result = database_engine_execute_read(ticket_select_by_user_identifier, 
        &ticket_create_from_query, &database_user_identifier, 1, sizeof(database_user_identifier), 
        1);

    if(result == NULL)
    {
        if(query_result == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            *error = query_result;
            return result;
        }

        database_engine_log_error("ticket_find_by_user_identifier", query_result);
        *error = query_result;
    }

    *error = (SHARED_OK);  
    return result; 
}

TicketCollection *
ticket_collection_find_by_flight_identifier(uint32_t flight_identifier, uint32_t *error)
{
      uint32_t database_flight_identifier = htonl(flight_identifier);

    void *result;
    uint32_t query_result = 0;

    result = database_engine_execute_read(ticket_select_by_flight_identifier, 
        &ticket_create_from_query, &database_flight_identifier, 1, 
        sizeof(database_flight_identifier), 1);

    if(result == NULL)
    {
        if(query_result == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            *error = query_result;
            return result;
        }

        database_engine_log_error("ticket_collection_find_by_flight_identifier", query_result);
        *error = query_result;
    }

    *error = (SHARED_OK);  
    return result;   
}

TicketCollection *
ticket_collection_find_by_ticket_identifier(uint32_t ticket_identifier, uint32_t *error)
{    
    uint32_t database_ticket_identifier = htonl(ticket_identifier);

    void *result;
    uint32_t query_result = 0;

    result = database_engine_execute_read(ticket_select_by_ticket_identifier, 
        &ticket_create_from_query, &database_ticket_identifier, 1, 
        sizeof(database_ticket_identifier), 1);

    if(result == NULL)
    {
        if(query_result == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            *error = query_result;
            return result;
        }

        database_engine_log_error("ticket_collection_find_by_ticket_identifier", query_result);
        *error = query_result;
    }

    *error = (SHARED_OK);  
    return result; 
}