#include "pages/partial/header/header_render.h"

#include <stdio.h>
#include <kore/kore.h>
#include <mustache.h>
#include <stdbool.h>

#include "shared/shared_error.h"
#include "assets.h"

int header_render(PartialContext *context);
void header_render_clean(PartialContext *context);
uintmax_t header_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
header_render(PartialContext *context)
{   
    int err = 0;

    if((err = partial_render_create_str_context(context, 
        (const char* const)asset_header_chtml)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    mustache_api_t api={
        .read = &partial_strread,  //std read will suffice
        .write = &partial_strwrite,     // need custom write for handling mustache_str_ctx **
        .varget = &header_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = partial_render_mustache_render(&api, context)) != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void 
header_render_clean(PartialContext *context)
{
    partial_render_clean(context);
}

uintmax_t
header_varget(mustache_api_t *api, void *userdata, mustache_token_variable_t *token)
{
    const char *output_string = NULL;
    bool isAdmin = true;
    bool isLoggedIn = true;
    
    if(strncmp("NAVBAR", token->text, token->text_length) == 0)
    {
        //TODO: acquire current logged in user role, if admin show admin nav
        if(!isAdmin)
        {
            output_string = "<a href='/'>Home</a>\n<a href=''>Flights</a>";
        }
        else
        {
            output_string = "<a href='/'>Home</a>\n<a href='flightsearch'>Flights</a>\n<a href='admin'>Admin</a>";
        }
    }
    else if(strncmp("LOGGEDIN", token->text, token->text_length) == 0)
    {
        //TODO: acquire current logged in user role, if admin show admin nav
        if(!isLoggedIn)
        {
            output_string = "<a href='login'>login/register</a>";
        }
        else
        {
            output_string = "<a href='userdetail'>User</a>\n<a href='logout'>Logout</a>";
        }
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "failed user list render: unknown template variable");
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