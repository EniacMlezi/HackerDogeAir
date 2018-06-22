
#include "pages/admin/givepoints/givepoints_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"

int         admin_give_points_render(UserContext *);
void        admin_give_points_render_clean(UserContext *);
void        admin_give_points_error(mustache_api_t *, void *, uintmax_t, char const *);

int
admin_give_points_render(UserContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &user_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = full_render((PartialContext *)context, &api, (const char* const)asset_givepoints_chtml))
     != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void
admin_give_points_render_clean(UserContext *context)
{
    partial_render_clean(&context->partial_context);
}

