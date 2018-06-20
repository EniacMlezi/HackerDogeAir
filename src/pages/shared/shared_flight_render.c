#include "pages/shared/shared_flight_render.h"

#include <kore/kore.h>
#include <mustache.h>

#include "shared/shared_error.h"
#include "shared/shared_time.h"

uintmax_t
flight_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
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
            char id_string[12];
            if(snprintf(id_string, 12, "%d", ctx->flight->id) <= 0)
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