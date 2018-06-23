
#include "pages/admin/bookinglist/bookinglist_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_booking_render.h"
#include "model/ticket.h"

int         admin_booking_list_render(BookingListContext *);
void        admin_booking_list_render_clean(BookingListContext *);
void        admin_booking_list_error(mustache_api_t *, void *, uintmax_t, char const *);

uintmax_t   admin_booking_list_sectget(mustache_api_t *, void *, mustache_token_section_t *);
//uintmax_t   admin_booking_list_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
admin_booking_list_render(BookingListContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &booking_varget,
        .sectget = &admin_booking_list_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_bookinglist_chtml))
     != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void
admin_booking_list_render_clean(BookingListContext *context)
{
    partial_render_clean(&context->partial_context);
}

/*
uintmax_t
admin_booking_list_varget(mustache_api_t *api, void *bookingdata, mustache_token_variable_t *token)
{
    BookingContext *ctx = (BookingContext *) bookingdata;
    const char *output_string = NULL;
    if(strncmp("userid", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->booking)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char userid_string[12];
            if(snprintf(userid_string, 12, "%d", ctx->booking->user_identifier) <= 0)
            {
                kore_log(LOG_ERR, 
                    "admin_booking_list_varget: failed int to string conversion for timeout. input: %d",
                    ctx->booking->user_identifier);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = userid_string;
        }
    } 
    else if(strncmp("flightid", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->booking)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char flightid_string[12];
            if(snprintf(flightid_string, 12, "%d", ctx->booking->flight_identifier) <= 0)
            {
                kore_log(LOG_ERR, 
                    "admin_booking_list_varget: failed int to string conversion for timeout. input: %d",
                    ctx->booking->flight_identifier);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = flightid_string;
        }
    }
    else if(strncmp("ticketid", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->booking)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char ticketid_string[12];
            if(snprintf(ticketid_string, 12, "%d", ctx->booking->ticket_identifier) <= 0)
            {
                kore_log(LOG_ERR, 
                    "admin_booking_list_varget: failed int to string conversion for timeout. input: %d",
                    ctx->booking->ticket_identifier);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = ticketid_string;
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "admin_booking_list_varget_varget: unknown template variable '%s'", token->text);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    ctx->partial_context.should_html_escape = true;
    if(api->write(api, bookingdata, output_string, strlen(output_string)) 
        != (SHARED_RENDER_MUSTACHE_OK))
    {
        kore_log(LOG_ERR, "admin_booking_list_varget_varget: failed to write");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}
*/

uintmax_t
admin_booking_list_sectget(mustache_api_t *api, void *bookingdata, mustache_token_section_t *token) 
{
    BookingListContext *ctx = (BookingListContext *) bookingdata;
    if (strcmp("bookingslist", token->name) == 0)
    {
        if(ctx->ticket_collection == NULL || TAILQ_EMPTY(ctx->ticket_collection))
        { 
            return (SHARED_RENDER_MUSTACHE_OK);
        }

        TicketCollectionNode *booking_node = NULL;
        BookingContext booking_context;
        memcpy(&booking_context.partial_context, &ctx->partial_context, sizeof(PartialContext));
        TAILQ_FOREACH(booking_node, ctx->ticket_collection, ticket_collection)
        {
            booking_context.ticket = booking_node->ticket;
            if(!mustache_render(api, &booking_context, token->section))
            {
                kore_log(LOG_ERR, "admin_booking_list_sectget: failed to render a booking");
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    kore_log(LOG_ERR, "admin_booking_list_sectget: unknown template section '%s'", token->name);
    return (SHARED_RENDER_MUSTACHE_FAIL);
}