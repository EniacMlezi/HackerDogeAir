#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/admin/userlist/userlist_render.h"
#include "model/user.h"
#include "model/role.h"
#include "assets.h"

int    admin_user_list(struct http_request *);
void   admin_user_list_error_handler(struct http_request *, int);

int 
admin_user_list(struct http_request *req)
{
    uint32_t err = (SHARED_OK);
    UserListContext context = {
        .partial_context = { .session_id = 0 }  //TODO: fill from request cookie
    };

    switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            context.user_collection = user_get_all_users(&err);

            if(context.user_collection == NULL)
            {
                switch(err)
                {
                    case (DATABASE_ENGINE_ERROR_NO_RESULTS):
                    case (SHARED_OK):
                        break;
                    default:
                       admin_user_list_error_handler(req, err);
                       goto exit; 
                }
            }

            if((err = admin_user_list_render(&context)) != (SHARED_OK))
            {
                admin_user_list_error_handler(req, err);
                goto exit;
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string,
                strlen(context.partial_context.dst_context->string));
            goto exit;
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET exist on this page
    }
exit:
    admin_user_list_render_clean(&context);
    return (KORE_RESULT_OK);
}

void
admin_user_list_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch(errcode) 
    {
        default:
            handled = false;
    }
    if (!handled) 
    {
        shared_error_handler(req, errcode, "/admin");
    }
}