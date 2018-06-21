#include "pages/user/user_bookings/user_bookings_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>
#include <sys/queue.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_flight_render.h"

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
        (const char* const)asset_user_bookings_chtml)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
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
        if (SLIST_EMPTY(&ctx->userbookinglist))
        {   //don's render the section, but don't crash either
            return (SHARED_RENDER_MUSTACHE_OK);
        }

        UserBookingListNode *user_booking_node = NULL;
        api->varget = &flight_varget;
        FlightContext flightcontext;
        memcpy(&flightcontext.partial_context, &ctx->partial_context, sizeof(PartialContext));
        SLIST_FOREACH(user_booking_node, &ctx->userbookinglist, userbookings)
        {
            flightcontext.flight = &user_booking_node->flight;
            if(!mustache_render(api, &flightcontext, token->section))
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