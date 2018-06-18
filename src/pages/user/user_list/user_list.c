#include <stdbool.h>
#include <limits.h>
#include <mustache.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/user/user_list/user_list_render.h"
#include "model/user.h"
#include "assets.h"

int     user_list(struct http_request *);

void    user_list_error_handler(struct http_request *req, int errcode);

int
user_list(struct http_request *req)
{
    int err = 0;
    UserListContext context = {
        .shared_context = {.session_id = 0}
    };

    //TODO: get Users from DataAccess layer
    SLIST_INIT(&context.userlist);
    char *email0 = "larsgardien@live.nl";
    char *email1 = "DennisSmith@live.nl";
    UserListNode user_node0 = {.user = {.id = 1, .email = email0}};
    UserListNode user_node1 = {.user = {.id = 2, .email = email1}};
    SLIST_INSERT_HEAD(&context.userlist, &user_node0, users);
    SLIST_INSERT_HEAD(&context.userlist, &user_node1, users);

    if(req->method == HTTP_METHOD_GET)
    {
        if((err = user_list_render(&context)) != (SHARED_ERROR_OK))
        {
            user_list_error_handler(req, err);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context.shared_context.dst_context->string, 
            strlen(context.shared_context.dst_context->string));

        user_list_render_clean(&context);
        return (KORE_RESULT_OK);
    }

    return (KORE_RESULT_ERROR);
}

void
user_list_error_handler(struct http_request *req, int errcode)
{
    // Users currently has no specific errors. Use generic handler.
    shared_error_handler(req, errcode);
}