#include "pages/flight/flight_search/flight_search_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>
#include <sys/queue.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "shared/shared_time.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_flight_render.h"

int         flight_search_render(FlightSearchContext *);
uintmax_t   flight_search_varget(mustache_api_t *, void *, mustache_token_variable_t *);
uintmax_t   flight_search_sectget(mustache_api_t *, void *, mustache_token_section_t *);

void        flight_search_render_clean(FlightSearchContext *);

int
flight_search_render(FlightSearchContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &flight_search_varget,
        .sectget = &flight_search_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api,
        (const char* const)asset_flight_search_chtml)) != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void         
flight_search_render_clean(FlightSearchContext *context)
{
    partial_render_clean(&context->partial_context);
}

uintmax_t
flight_search_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    uint32_t err = 0;
    FlightSearchContext *ctx = (FlightSearchContext *) userdata;
    char date_conversion_ouput[30];
    const char *output_string = NULL;

    if(strncmp("arrivaldate", token->text, token->text_length) == 0)
    {
        time_t epoch;
        if(shared_time_tm_to_epoch(&ctx->params.arrivaldate, &epoch) != (SHARED_OK))
        {
            kore_log(LOG_ERR, "flight_search_varget: time conversion error %d", err);
        }

        if(epoch == -1 || epoch == 0)
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
            err = shared_time_tm_to_string(&ctx->params.arrivaldate, date_conversion_ouput,
                    sizeof(date_conversion_ouput), "%d-%m-%Y");

            if(err == (SHARED_ERROR_TIME_CONVERSION))
            {
                kore_log(LOG_ERR, "flight_search_varget: time conversion error %d", err);
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
        kore_log(LOG_INFO, "flight_search_varget: unknown template variable: %s", token->text);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    size_t output_string_len = strlen(output_string);
    uintmax_t ret = api->write(api, userdata, output_string, output_string_len);
    if(ret != output_string_len)
    {
        kore_log(LOG_ERR, "flight_search_varget: failed to write. wrote: %ld, expected: %ld",
            ret, output_string_len);
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
        api->varget = &flight_varget;
        FlightContext flightcontext;
        memcpy(&flightcontext.partial_context, &ctx->partial_context, sizeof(PartialContext));
        SLIST_FOREACH(flight_node, &ctx->flightlist, flights)
        {
            flightcontext.flight = &flight_node->flight;
            if(!mustache_render(api, &flightcontext, token->section))
            {
                kore_log(LOG_ERR, "flight_search_sectget: failed to render a flight");
                api->varget = &flight_search_varget;
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
        }
        api->varget = &flight_search_varget;
        return (SHARED_RENDER_MUSTACHE_OK);
    }
    kore_log(LOG_ERR, "flight_search_sectget: unknown template section '%s'", token->name);
    return (SHARED_RENDER_MUSTACHE_FAIL);
}