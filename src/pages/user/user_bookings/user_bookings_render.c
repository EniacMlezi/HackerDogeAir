#include "pages/user/user_bookings/user_bookings_render.h"
#include "pages/admin/bookinglist/bookinglist_render.h"
#include "pages/shared/shared_booking_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>
#include <sys/queue.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_booking_render.h"

int         user_bookings_render(UserBookingsContext *);
uintmax_t   user_bookings_varget(mustache_api_t *, void *, mustache_token_variable_t *);
uintmax_t   user_bookings_sectget(mustache_api_t *, void *, mustache_token_section_t *);

void        user_bookings_render_clean(UserBookingsContext *);

int
user_bookings_render(UserBookingsContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &partial_varget,
        .sectget = &user_bookings_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api,
        (const char* const)asset_user_bookings_chtml)) != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void         
user_bookings_render_clean(UserBookingsContext *context)
{
    partial_render_clean(&context->partial_context);
}

uintmax_t
user_bookings_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token)
{
    UserBookingsContext *ctx = (UserBookingsContext *)userdata;

    if(strcmp("userbookings", token->name) == 0)
    {
        if (ctx->ticket_collection == NULL || TAILQ_EMPTY(ctx->ticket_collection))
        {   //don's render the section, but don't crash either
            return (SHARED_RENDER_MUSTACHE_OK);
        }

        TicketCollectionNode *ticket_node = NULL;
        api->varget = &booking_varget;
        BookingContext ticket_context;
        memcpy(&ticket_context.partial_context, &ctx->partial_context, sizeof(PartialContext));
        TAILQ_FOREACH(ticket_node, ctx->ticket_collection, ticket_collection)
        {
            ticket_context.ticket = ticket_node->ticket;
            if(!mustache_render(api, &ticket_context, token->section))
            {
                kore_log(LOG_ERR, "user_bookings_sectget: failed to render a userbooking");
                api->varget = &partial_varget;
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        api->varget = &partial_varget;
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    kore_log(LOG_ERR, "user_bookings_sectget: unknown template section '%s'", token->name);
    return (SHARED_RENDER_MUSTACHE_FAIL);
}