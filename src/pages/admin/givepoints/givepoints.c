#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/admin/givepoints/givepoints_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"
#include "assets.h"

int         admin_give_points(struct http_request *);

uint32_t    admin_give_points_parse_params(struct http_request *, User *, bool);
uint32_t    admin_give_points_find_user(User **);

void        admin_give_points_error_handler(struct http_request *, int);


int 
admin_give_points(struct http_request *req)
{
    int err;
    User user = (User) {
        .identifier = 0, 
        .role = 0, 
        .email = NULL, 
        .username = NULL, 
        .first_name = NULL,
        .last_name =  NULL, 
        .telephone_number = NULL, 
        .password = NULL,
        .doge_coin = 0, 
        .registration_datetime = (struct tm) {
            .tm_year     = 0,
            .tm_mon      = 0,
            .tm_mday     = 0,
            .tm_hour     = 0,
            .tm_min      = 0,
            .tm_sec      = 0,
            .tm_wday     = 0,
            .tm_yday     = 0
        }};

    UserContext context = {
        .partial_context = { .session_id = 0 }, //TODO: fill from request cookie
        .user = &user
    };

    if((err = admin_give_points_parse_params(req, context.user, false)) != (SHARED_OK)) 
    {
        admin_give_points_error_handler(req, err);
        return(KORE_RESULT_OK);
    }
    kore_log(LOG_ERR, "admin_give_points: id param %i", context.user->identifier);


    if ((err = admin_give_points_find_user(&context.user)) != (SHARED_OK))
    {
        admin_give_points_error_handler(req, err);
        return(KORE_RESULT_OK);
    }

    kore_log(LOG_ERR, "admin_give_points: username database %s", context.user->username);
    kore_log(LOG_ERR, "admin_give_points: id database %i", context.user->identifier);


    switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            if((err = admin_give_points_render(&context)) != (SHARED_OK))
            {
                admin_give_points_error_handler(req, err);
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string,
                strlen(context.partial_context.dst_context->string));

            admin_give_points_render_clean(&context);
            user_destroy(&context.user);
            return (KORE_RESULT_OK);
        }

        case HTTP_METHOD_POST:
        {
            if((err = admin_give_points_parse_params(req, context.user, true)) != (SHARED_OK))
            {
                admin_give_points_error_handler(req, err);
            }
            kore_log(LOG_ERR, "admin_give_points: username %s", context.user->username);

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, asset_login_success_html, asset_len_login_success_html);

            user_destroy(&context.user);
            return (KORE_RESULT_OK);
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET and POST exist on this page
    }    
}

uint32_t 
admin_give_points_parse_params(struct http_request *req, User *user, bool post)
{
    int err = (SHARED_OK);
    if (!post) //get
    {
        http_populate_get(req);
        if(!http_argument_get_int32(req, "id", &(user->identifier)))
        {
            err = (ADMIN_GIVE_POINTS_ID_INVALID);
        }
    }
    else
    {
        http_populate_post(req);
        if(!http_argument_get_int32(req, "dogecoins", &(user->doge_coin)))
        {
            err = (ADMIN_GIVE_POINTS_INTEGER_CONVERSION_ERROR);
        }
    }  

    return err;
}

uint32_t    
admin_give_points_find_user(User **user)
{
    uint32_t err;         /* Error variable for the called functions. */
    User *database_user;

    database_user = user_find_by_identifier((*user)->identifier, &err);
    if(database_user == NULL)
    {
        if(err == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            err = (ADMIN_GIVE_POINTS_ID_NOT_FOUND);
        }
        return err;
    }

    *user = database_user;
    return (SHARED_OK);

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
        case (ADMIN_GIVE_POINTS_INTEGER_CONVERSION_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR, 
                "Invalid integer conversion. Please try again.", "/admin/userlist", 10);
            break;

        default:
            handled = false;
    }
    if (!handled) 
    {
        shared_error_handler(req, errcode, "");
    }
}