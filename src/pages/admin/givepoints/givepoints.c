#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/admin/givepoints/givepoints_render.h"
#include "model/user.h"
#include "assets.h"

int         admin_give_points(struct http_request *);

uint32_t    admin_give_points_find_user(User *);
uint32_t    admin_give_points_parse_params(struct http_request *, int *);

void        admin_give_points_error_handler(struct http_request *, int);


int 
admin_give_points(struct http_request *req)
{
    int err;
    User user;

    UserContext context = {
        .partial_context = { .session_id = 0 }, //TODO: fill from request cookie
        .user = &user
    };

    int userid = 0;
    if((err = admin_give_points_parse_params(req, &userid)) != (SHARED_OK)) 
    {
        admin_give_points_error_handler(req, err);
    }
    kore_log(LOG_ERR, "admin_give_points: id %i", userid);

    user.identifier = userid;

    kore_log(LOG_ERR, "admin_give_points: id %i", context.user->identifier);


    /*if ((err = admin_give_points_find_user(&user)) != (SHARED_OK))
    {
        admin_give_points_error_handler(req, err);
    }*/


    /*switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            if((err = admin_give_points_render(&context)) != (SHARED_OK))
            {
                admin_give_points_error_handler(req, err);
            }
        }

        case HTTP_METHOD_POST:
        {
            return (KORE_RESULT_OK);
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET and POST exist on this page
    }*/

    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }
    
    //a GET receives the home form and renders the page
    if((err = admin_give_points_render(&context)) != (SHARED_OK))
    {
        admin_give_points_error_handler(req, err);
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
        context.partial_context.dst_context->string,
        strlen(context.partial_context.dst_context->string));

    admin_give_points_render_clean(&context);
    return (KORE_RESULT_OK);    
}

uint32_t    
admin_give_points_find_user(User *user)
{
    uint32_t err;         /* Error variable for the called functions. */
    User *database_user;

    database_user = user_find_by_identifier(user->identifier, &err);
    if(database_user == NULL)
    {
        if(err == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            err = (ADMIN_GIVE_POINTS_ID_NOT_FOUND);
        }
        return err;
    }

    user = database_user;
    return (SHARED_OK);

}

uint32_t 
admin_give_points_parse_params(struct http_request *req, int *userid)
{
    http_populate_get(req);
    int err = (SHARED_OK);
    if(!http_argument_get_int32(req, "id", userid))
    {
        err = (ADMIN_GIVE_POINTS_ID_INVALID);
    }

    return err;
}

void
admin_give_points_error_handler(struct http_request *req, int errcode)
{
    bool handled = true;
    switch(errcode) 
    {
        case (ADMIN_GIVE_POINTS_ID_INVALID):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Invalid User Identifier. Please try again.", "/admin/userlist", 10);
            break;

        case (ADMIN_GIVE_POINTS_ID_NOT_FOUND):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Unknown User Identifier. Please try again.", "/admin/userlist", 10);
            break;

        default:
            handled = false;
    }
    if (!handled) 
    {
        shared_error_handler(req, errcode, "");
    }
}