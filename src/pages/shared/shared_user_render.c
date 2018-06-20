#include "pages/shared/shared_user_render.h"

#include <kore/kore.h>
#include <mustache.h>

uintmax_t
user_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    UserContext *ctx = (UserContext *) userdata;
    const char *output_string = NULL;
    if(strncmp("id", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            char id_string[12];
            if(snprintf(id_string, 12, "%d", ctx->user->id) <= 0)
            {
                kore_log(LOG_ERR, 
                    "user_varget: failed int to string conversion for timeout. input: %d",
                    ctx->user->id);
                return (SHARED_RENDER_MUSTACHE_FAIL);
            }
            output_string = id_string;
        }
    }
    else if(strncmp("email", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user || NULL == ctx->user->email)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->user->email;
        }
    }
    else if (strncmp("firstname", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user || NULL == ctx->user->firstname)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->user->firstname;
        }
    }
    else if (strncmp("lastname", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user || NULL == ctx->user->lastname)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->user->lastname;
        }
    }
    else if (strncmp("username", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user || NULL == ctx->user->username)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->user->username;
        }
    }    
    else if (strncmp("telnumber", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->user || NULL == ctx->user->telnumber)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->user->telnumber;
        }
    }
    else if(strncmp("error_message", token->text, token->text_length) == 0)
    {
        if(NULL == ctx->error_message)
        {
            output_string = (SHARED_RENDER_EMPTY_STRING);
        }
        else
        {
            output_string = ctx->error_message;
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "user_varget: unknown template variable '%s'", token->text);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    uintmax_t output_string_len = strlen(output_string);
    uintmax_t ret = api->write(api, userdata, output_string, output_string_len);
    if(ret != output_string_len)
    {
        kore_log(LOG_ERR, "user_varget: failed to write. wrote: %ld, expected: %ld", 
            ret, output_string_len);
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}