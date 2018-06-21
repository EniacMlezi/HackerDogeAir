#include "pages/user/user_detail/user_detail_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"

int         user_detail_render(UserContext *);
void        user_detail_render_clean(UserContext *);

int
user_detail_render(UserContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &user_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_user_detail_chtml)) 
        != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void         
user_detail_render_clean(UserContext *context)
{
    partial_render_clean(&context->partial_context);
}