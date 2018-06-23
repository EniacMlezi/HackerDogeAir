#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/admin/givepoints/givepoints_render.h"
#include "pages/shared/shared_user_render.h"
#include "model/user.h"
#include "model/session.h"
#include "assets.h"

typedef struct AdminGivePointsParams
{
    uint32_t user_identifier;
    uint32_t doge_coins;
} AdminGivePointsParams;

int         admin_give_points(struct http_request *);

uint32_t    admin_give_points_parse_params(struct http_request *, AdminGivePointsParams *, bool);
uint32_t    admin_give_points_find_user(User **);

void        admin_give_points_error_handler(struct http_request *, int);

int 
admin_give_points(struct http_request *req)
{
    if(req->method != HTTP_METHOD_GET && req->method != HTTP_METHOD_POST)
    {
        return (KORE_RESULT_ERROR);
    }

    uint32_t err;
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

    Session session = (Session) {
        .identifier = NULL,
        .user_identifier = 0
    };

    UserContext context = {
        .partial_context = { 
            .src_context = NULL,
            .dst_context = NULL,
            .session = &session 
        },
        .user = &user
    };

    if ((err = shared_http_find_session_from_request(
        req, &context.partial_context.session)) != (SHARED_OK))
    {
        admin_give_points_error_handler(req, err);
        return(KORE_RESULT_OK);
    }

    AdminGivePointsParams params = { 0, 0 };

    if((err = admin_give_points_parse_params(req, &params, req->method == HTTP_METHOD_POST)) 
        != (SHARED_OK))
    {
        admin_give_points_error_handler(req, err);
        goto exit;
    }

    User *db_user;
    db_user = user_find_by_identifier(params.user_identifier, &err);
    if(db_user == NULL)
    {
        if(err == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            err = (ADMIN_GIVE_POINTS_ID_NOT_FOUND);
        }
        admin_give_points_error_handler(req, err);
        goto exit;
    }

    switch(req->method)
    {
        case HTTP_METHOD_GET:
        {
            context.user->identifier = db_user->identifier;
            context.user->username = db_user->username;
            context.user->doge_coin = db_user->doge_coin;
            if((err = admin_give_points_render(&context)) != (SHARED_OK))
            {
                admin_give_points_error_handler(req, err);
                goto exit;
            }

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                context.partial_context.dst_context->string,
                strlen(context.partial_context.dst_context->string));
            goto exit;
        }

        case HTTP_METHOD_POST:
        {
            db_user->doge_coin = params.doge_coins;
            user_update_coins(db_user);

            http_response_header(req, "content-type", "text/html");
            http_response(req, HTTP_STATUS_OK, 
                asset_givepoints_success_html, asset_len_givepoints_success_html);

            goto exit;
        }

        default:
            return(KORE_RESULT_ERROR); //No methods besides GET and POST exist on this page
    }  

exit:
    admin_give_points_render_clean(&context);
    user_destroy(&db_user);
    return (KORE_RESULT_OK);
}

uint32_t 
admin_give_points_parse_params(struct http_request *req, AdminGivePointsParams *params, bool post)
{
    int err = (SHARED_OK);
    if (!post) //get
    {
        http_populate_get(req);
        if(!http_argument_get_int32(req, "id", &params->user_identifier))
        {
            err = (ADMIN_GIVE_POINTS_ID_INVALID);
        }
    }
    else
    {
        http_populate_post(req);
        if(!http_argument_get_int32(req, "id", &params->user_identifier))
        {
            err = (ADMIN_GIVE_POINTS_ID_INVALID);
        }
        if(!http_argument_get_int32(req, "dogecoins", &params->doge_coins))
        {
            err = (ADMIN_GIVE_POINTS_COINS_INVALID);
        }
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
        case (ADMIN_GIVE_POINTS_COINS_INVALID):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Invalid Coins", "/admin/userlist", 10);
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