#include "pages/flight/flight_search/flight_search_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>
#include <sys/queue.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"
#include "pages/partial/partial_render.h"

int         flight_search_render(FlightSearchContext *);
uintmax_t   flight_search_varget(mustache_api_t *, void *, mustache_token_variable_t *);
uintmax_t   flight_search_varget_flights(mustache_api_t *, void *, mustache_token_variable_t *);
uintmax_t   flight_search_sectget(mustache_api_t *, void *, mustache_token_section_t *);

void        flight_search_render_clean(FlightSearchContext *);

int
flight_search_render(FlightSearchContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &flight_search_varget,
        .sectget = &flight_search_sectget,
        .error = &shared_error,
    };

    if((err = shared_render((SharedContext *)context, &api,
        (const char* const)asset_flight_search_chtml)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void         
flight_search_render_clean(FlightSearchContext *context)
{
    shared_render_clean(&context->shared_context);
}

uintmax_t
flight_search_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    int err = 0;
    FlightSearchContext *ctx = (FlightSearchContext *) userdata;
    char date_conversion_ouput[30];
    const char *output_string = NULL;

    if(strncmp("arrivaldate", token->text, token->text_length) == 0)
    {
        if(ctx->params.arrivaldate == 0)
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
            if ((err = shared_time_time_t_to_string(
                &ctx->params.arrivaldate,
                date_conversion_ouput,
                "%d-%m-%Y",
                sizeof(date_conversion_ouput)) != (SHARED_ERROR_OK)))
            {
                kore_log(LOG_INFO, "time conversion error %d", err);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }

            output_string = date_conversion_ouput;
        }
    }

    else if(strncmp("results_hidden", token->text, token->text_length) == 0)
    {
        if (SLIST_EMPTY(&ctx->flightlist))
        {
            output_string = SHARED_RENDER_HIDDEN_STRING;
        }
        else
        {
            output_string = SHARED_RENDER_INVALID_STRING;
        }
    }

    else if(strncmp("error_message", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->error_message)
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
            output_string = ctx->error_message;
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "failed flight_search render: unknown template variable: %s", token->text);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    size_t output_string_len = strlen(output_string);
    uintmax_t ret = api->write(api, userdata, output_string, output_string_len);
    if(ret != output_string_len)
    {
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}

uintmax_t
flight_search_varget_flights(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    int err = 0;
    FlightContext *ctx = (FlightContext *) userdata;
    const char *output_string = NULL;
    char date_conversion_ouput[30];

    if(strncmp("id", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->flight)
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
            char id_string[11];
            if(snprintf(id_string, 11, "%d", ctx->flight->id) <= 0)
            {
                kore_log(LOG_INFO, "flights id failed");
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = id_string;
        }
    }

    else if(strncmp("arrivaldatetime", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->flight || ctx->flight->arrival_datetime == 0)
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
            if ((err = shared_time_time_t_to_string(
                &ctx->flight->arrival_datetime,
                date_conversion_ouput,
                "%d-%m-%Y %T",
                sizeof(date_conversion_ouput)) != (SHARED_ERROR_OK)))
            {
                kore_log(LOG_INFO, "time conversion error %d", err);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = date_conversion_ouput;
        }
    }

    else if(strncmp("departuredatetime", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->flight || ctx->flight->departure_datetime == 0)
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
            if ((err = shared_time_time_t_to_string(
                &ctx->flight->departure_datetime,
                date_conversion_ouput,
                "%d-%m-%Y %T",
                sizeof(date_conversion_ouput)) != (SHARED_ERROR_OK)))
            {
                kore_log(LOG_INFO, "time conversion error %d", err);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = date_conversion_ouput;
        }
    }
    
    else if(strncmp("arrivallocation", token->text, token->text_length))
    { 
        if(NULL == ctx->flight || NULL == ctx->flight->arrival_location)
        {
            output_string = SHARED_RENDER_EMPTY_STRING; 
        }
        else
        {
            output_string = ctx->flight->arrival_location;
        }
    }

    else if(strncmp("departurelocation", token->text, token->text_length))
    {
        if(NULL == ctx->flight || NULL == ctx->flight->departure_location)
        {
            output_string = SHARED_RENDER_EMPTY_STRING; 
        }
        else
        {
            output_string = ctx->flight->departure_location;
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "failed flight_search flights render: unknown template variable: %s", token->text);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    size_t output_string_len = strlen(output_string);
    uintmax_t ret = api->write(api, userdata, output_string, output_string_len);
    if(ret != output_string_len)
    {
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}

uintmax_t
flight_search_sectget(mustache_api_t *api, void *userdata, mustache_token_section_t *token)
{
    FlightSearchContext *ctx = (FlightSearchContext *)userdata;

    if(strcmp("flights", token->name) == 0)
    {
        if (SLIST_EMPTY(&ctx->flightlist))
        { 
            return (SHARED_RENDER_MUSTACHE_OK);
        }

        FlightSearchListNode *flight_node = NULL;
        api->write = &shared_mustache_strwrite;
        api->varget = &flight_search_varget_flights;
        SLIST_FOREACH(flight_node, &ctx->flightlist, flights)
        {
            FlightContext flightcontext = {
                .dst_context = ctx->shared_context.dst_context,
                .flight = &flight_node->flight
            };
            if(!mustache_render(api, &flightcontext, token->section))
            {
                api->write = &shared_strwrite;
                api->varget = &flight_search_varget;
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        api->write = &shared_strwrite;
        api->varget = &flight_search_varget;
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    return (SHARED_RENDER_MUSTACHE_FAIL);
}