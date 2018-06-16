#include <stdbool.h>
#include <limits.h>
#include <mustache.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/user/user_render.h"
#include "model/user.h"
#include "assets.h"

//TODO: verify numbers aren't overlapping
#define USER_ERROR_ID_VALIDATOR_INVALID 600
#define USER_ERROR_ID_INCORRECT         601

int     user(struct http_request *);
int     user_parseparams(struct http_request *, int *);
int     user_query(int, User *);

void    user_error_handler(struct http_request *req, int errcode);

int
user(struct http_request *req)
{
    int err = 0;
    char *email = "larsgardien@live.nl";
    User user = {25, email, NULL};
    UserContext context = {
        .shared_context = {.session_id = 0},
        .user = &user
    };
    if(req->method == HTTP_METHOD_GET)
    {
        int userid;
        if((err = user_parseparams(req, &userid)) != (SHARED_ERROR_OK))
        {
            user_error_handler(req, err);
            return (KORE_RESULT_OK);
        }

        //TODO: fill context.user with DataAccess Layer

        if((err = user_render(&context)) != (SHARED_ERROR_OK))
        {
            user_error_handler(req, err);
            return (KORE_RESULT_OK);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context.shared_context.dst_context->string, 
            strlen(context.shared_context.dst_context->string));

        user_render_clean(&context);
        return (KORE_RESULT_OK);
    }
    
    return (KORE_RESULT_ERROR);
}

int    
user_parseparams(struct http_request *req, int *userid)
{
    http_populate_get(req);
    if(!http_argument_get_uint32(req, "id", userid))
    {
        return (USER_ERROR_ID_VALIDATOR_INVALID); 
    }
    return (SHARED_ERROR_OK);
}

void
user_error_handler(struct http_request *req, int errcode)
{
    kore_log(LOG_INFO, "user render error");
}