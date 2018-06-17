
#include "pages/home/home_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/shared/shared_render.h"
#include "model/user.h"

int         home_render(SharedContext *);
void        home_render_clean(SharedContext *);
void        home_error(mustache_api_t *, void *, uintmax_t, char const *);

int
home_render(SharedContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &shared_strread,
        .write = &shared_strwrite,
        .varget = &shared_varget,
        .sectget = &shared_sectget,
        .error = &shared_error,
    };

    if((err = shared_render(context, &api, (const char* const)asset_home_chtml))
     != (SHARED_ERROR_OK))
    {
        return err;
    }

    return (SHARED_ERROR_OK);
}

void
home_render_clean(SharedContext *context)
{
    shared_render_clean(context);
}