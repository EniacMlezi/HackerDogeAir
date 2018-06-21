#include "pages/user/user_actions_render.h"

#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"

int         user_actions_render(PartialContext *);
void        user_actions_render_clean(PartialContext *);

int
user_actions_render(PartialContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &partial_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = full_render(context, &api, (const char* const)asset_user_chtml)) 
        != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void
user_actions_render_clean(PartialContext *context)
{
    partial_render_clean(context);
}