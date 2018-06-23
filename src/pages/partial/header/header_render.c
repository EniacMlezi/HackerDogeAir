#include "pages/partial/header/header_render.h"

#include <stdio.h>
#include <kore/kore.h>
#include <mustache.h>
#include <stdbool.h>

#include "shared/shared_error.h"
#include "model/user.h"
#include "model/role.h"
#include "assets.h"

int header_render(PartialContext *context);
void header_render_clean(PartialContext *context);
uintmax_t header_varget(mustache_api_t *, void *, mustache_token_variable_t *);

int
header_render(PartialContext *context)
{   
    int err = 0;

    if((err = partial_render_create_str_context(context, 
        (const char* const)asset_header_chtml)) != (SHARED_OK))
    {
        return err;
    }

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &header_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = partial_render_mustache_render(&api, context)) != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
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
    uint32_t err = (SHARED_OK);

    PartialContext *ctx = (PartialContext *)userdata;

    User *user = NULL;
    if (ctx->session != NULL) {
        if (ctx->session->identifier != NULL) {
            user = user_find_by_session_identifier(ctx->session->identifier, &err);
        }
    }
    
    if(strncmp("NAVBAR", token->text, token->text_length) == 0)
    {
        if(ctx->session != NULL && user != NULL)
        {
            if (user->role == ADMIN) 
            {
                output_string = "<a href='/'>Home</a>\n<a href='/flight/search'>Flights</a>\n<a href='/admin'>Admin</a>";
            } 
            else
            {
                
                output_string = "<a href='/'>Home</a>\n<a href='/flight/search'>Flights</a>";
            }
        }
        else
        {
            output_string = "<a href='/'>Home</a>\n<a href='/flight/search'>Flights</a>";
        }
    }
    else if(strncmp("LOGGEDIN", token->text, token->text_length) == 0)
    {
        if(ctx->session != NULL && user != NULL)
        {
            output_string = "<a href='/user'>User</a>\n<a href='/logout'>Logout</a>";           
        }
        else
        {
            output_string = "<a href='login'>login/register</a>";
        }
    }
    ctx->should_html_escape = false;
    if(api->write(api, userdata, output_string, strlen(output_string)) != (SHARED_RENDER_MUSTACHE_OK))
    {
        kore_log(LOG_ERR, "header_varget: failed to write.");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }

    if(NULL == output_string)
    {
        kore_log(LOG_INFO, "failed user list render: unknown template variable");
        return (SHARED_RENDER_MUSTACHE_FAIL);
    }
    return (SHARED_RENDER_MUSTACHE_OK);
}