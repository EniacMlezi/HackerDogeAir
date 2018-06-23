#include "pages/admin/flightlist/flightlist_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>
#include <sys/queue.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_flight_render.h"

int         admin_flight_list_render(FlightListContext *);
void        admin_flight_list_render_clean(FlightListContext *);
void        admin_flight_list_error(mustache_api_t *, void *, uintmax_t, char const *);

uintmax_t   admin_flight_list_sectget(mustache_api_t *, void *, mustache_token_section_t *);

int
admin_flight_list_render(FlightListContext *context)
{
    int err = 0;
    
    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &partial_varget,
        .sectget = &admin_flight_list_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_flightlist_chtml))
     != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void
admin_flight_list_render_clean(FlightListContext *context)
{
    partial_render_clean(&context->partial_context);
}

uintmax_t
admin_flight_list_sectget(mustache_api_t *api, void *flightdata, mustache_token_section_t *token) 
{
    FlightListContext *ctx = (FlightListContext *) flightdata;
    if (strcmp("flightlist", token->name) == 0)
    {
        if (ctx->flight_collection == NULL || TAILQ_EMPTY(ctx->flight_collection))
        { 
            return (SHARED_RENDER_MUSTACHE_OK);
        }

        FlightCollectionNode *flight_node = NULL;
        api->varget = &flight_varget;
        FlightContext flight_context;
        memcpy(&flight_context.partial_context, &ctx->partial_context, sizeof(PartialContext));
        TAILQ_FOREACH(flight_node, ctx->flight_collection, flight_collection)
        {
            flight_context.flight = flight_node->flight;
            if(!mustache_render(api, &flight_context, token->section))
            {
                kore_log(LOG_ERR, "admin_flight_list_sectget: failed to render a flight");
                api->varget = &partial_varget;
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        api->varget = &partial_varget;
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    kore_log(LOG_ERR, "admin_flight_list_sectget: unknown template section '%s'", token->name);
    return (SHARED_RENDER_MUSTACHE_FAIL);
}
