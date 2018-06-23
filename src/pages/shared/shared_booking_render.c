#include "pages/shared/shared_booking_render.h"

#include <kore/kore.h>
#include <mustache.h>

#include "shared/shared_error.h"
#include "shared/shared_time.h"

uintmax_t
booking_varget(mustache_api_t *api,  void *booking_data, mustache_token_variable_t *token)
{
    BookingContext *ctx = (BookingContext *) booking_data;

    const char *output_string = NULL;
    if(strncmp("userid", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->ticket)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char userid_string[12];
            if(snprintf(userid_string, 12, "%d", ctx->ticket->user_identifier) <= 0)
            {
                kore_log(LOG_ERR, 
                    "booking_varget: failed int to string conversion for userid. input: %d",
                    ctx->ticket->user_identifier);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = userid_string;
        }
    } 
    else if(strncmp("flightid", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->ticket)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char flightid_string[12];
            if(snprintf(flightid_string, 12, "%d", ctx->ticket->flight_identifier) <= 0)
            {
                kore_log(LOG_ERR, 
                    "booking_varget: failed int to string conversion for flight_identifier. input: %d",
                    ctx->ticket->flight_identifier);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = flightid_string;
        }
    }
    else if(strncmp("ticketid", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->ticket)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char ticketid_string[12];
            if(snprintf(ticketid_string, 12, "%d", ctx->ticket->ticket_identifier) <= 0)
            {
                kore_log(LOG_ERR, 
                    "booking_varget: failed int to string conversion for ticket_identifier. input: %d",
                    ctx->ticket->ticket_identifier);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = ticketid_string;
        }
    }
    else if(strncmp("cost", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->ticket)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char cost_string[12];
            if(snprintf(cost_string, 12, "%d", ctx->ticket->cost) <= 0)
            {
                kore_log(LOG_ERR, 
                    "booking_varget: failed int to string conversion for cost. input: %d",
                    ctx->ticket->cost);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = cost_string;
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "booking_varget: unknown template variable '%s'", token->text);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    ctx->partial_context.should_html_escape = true;
    if(api->write(api, booking_data, output_string, strlen(output_string)) 
        != (SHARED_RENDER_MUSTACHE_OK))
    {
        kore_log(LOG_ERR, "booking_varget: failed to write");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}