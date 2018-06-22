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
    int err;
    UserListContext context = {
        .partial_context = { .session_id = 0 }  //TODO: fill from request cookie
    };
    SLIST_INIT(&context.userlist);

    switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            UserListNode user_node0 = {
                .user = {
                    .identifier = 0,
                    .email = "dennis17-17@hotmail.com",
                    .username = "EniacBlue",
                    .role = 2,
                    .doge_coin = 12000
                }
            };
            
            UserListNode user_node1 = {
                .user = {
                    .identifier = 1,
                    .email = "nick.devisser@hotmail.com",
                    .username = "EniacMazamo",
                    .role = 1,
                    .doge_coin = 10
                }
            };

            UserListNode user_node2 = {
                .user = {
                    .identifier = 2,
                    .email = "larsgardien@live.nl",
                    .username = "EniacMlezi",
                    .role = 1,
                    .doge_coin = 1022
                }
            };
            SLIST_INSERT_HEAD(&context.userlist, &user_node0, users);
            SLIST_INSERT_HEAD(&context.userlist, &user_node1, users);
            SLIST_INSERT_HEAD(&context.userlist, &user_node2, users);

            if((err = admin_user_list_render(&context)) != (SHARED_OK))
            {
                admin_user_list_error_handler(req, err);
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string,
                strlen(context.partial_context.dst_context->string));

            admin_user_list_render_clean(&context);
            return (KORE_RESULT_OK);
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET exist on this page
    }
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
        shared_error_handler(req, errcode, "");
    }
}