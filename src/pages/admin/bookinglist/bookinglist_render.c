
#include "pages/admin/bookinglist/bookinglist_render.h"

#include <stdbool.h>
#include <kore/kore.h>
#include <mustache.h>

#include "assets.h"
#include "shared/shared_error.h"
#include "pages/partial/partial_render.h"
#include "model/user.h"

int         admin_booking_list_render(PartialContext *);
void        admin_booking_list_render_clean(PartialContext *);
void        admin_booking_list_error(mustache_api_t *, void *, uintmax_t, char const *);

int
admin_booking_list_render(PartialContext *context)
{
    int err = 0;

    mustache_api_t api={
        .read = &partial_strread,
        .write = &partial_strwrite,
        .varget = &partial_varget,
        .sectget = &partial_sectget,
        .error = &partial_error,
    };

    if((err = full_render(context, &api, (const char* const)asset_home_chtml))
     != (SHARED_OK))
    {
        return err;
    }

    return (SHARED_OK);
}

void
admin_booking_list_render_clean(PartialContext *context)
{
    partial_render_clean(context);
}