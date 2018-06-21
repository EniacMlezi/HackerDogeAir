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
            if(snprintf(id_string, 12, "%d", ctx->flight->flight_identifier) 
                <= 0)
            {
                kore_log(LOG_ERR, "flight_varget: failed int to string conversion for flightid."\
                    " input: %d", ctx->flight->flight_identifier);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = id_string;
        }
    }

    else if(strncmp("arrivaldatetime", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->flight) 
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
           if(strftime(date_conversion_ouput,
            sizeof(date_conversion_ouput), "%d-%m-%Y %T", 
            &ctx->flight->arrival_datetime) == (SHARED_OK))
            {
                kore_log(LOG_ERR, "flight_varget: time conversion error %d", err);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }

            output_string = date_conversion_ouput;
        }
    }

    else if(strncmp("departuredatetime", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->flight)
        {
            output_string = SHARED_RENDER_EMPTY_STRING;
        }
        else
        {
            if(strftime(date_conversion_ouput,
            sizeof(date_conversion_ouput), "%d-%m-%Y %T", 
            &ctx->flight->departure_datetime) == (SHARED_OK))
            {
                kore_log(LOG_ERR, "flight_varget: time conversion error %d", err);
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
        kore_log(LOG_INFO, "flight varget: unknown template variable: '%s'", token->text);
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