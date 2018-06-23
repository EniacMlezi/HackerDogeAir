#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/admin/admin_render.h"
#include "model/user.h"
#include "model/session.h"
#include "assets.h"

int    admin(struct http_request *);
void   admin_error_handler(struct http_request *, int);

int 
admin(struct http_request *req)
{
    int err;
    Session session = (Session) {
        .identifier = NULL,
        .user_identifier = 0
    };
    PartialContext context = {
        .session = &session  //TODO: fill from request cookie
    };

    if ((err = shared_http_find_session_from_request(req, &context.session)) != (SHARED_OK))
    {
        admin_error_handler(req, err);
    }

    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }
    
    //a GET receives the home form and renders the page
    if((err = admin_render(&context)) != (SHARED_OK))
    {
        admin_error_handler(req, err);
        goto exit;
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
        context.dst_context->string,
        strlen(context.dst_context->string));
    
exit:
    admin_render_clean(&context);
    return (KORE_RESULT_OK);    
}

void
admin_error_handler(struct http_request *req, int errcode)
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