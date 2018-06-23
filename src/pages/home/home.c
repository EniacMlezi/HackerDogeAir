#include "pages/home/home.h"

#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "shared/shared_http.h"
#include "pages/home/home_render.h"
#include "model/user.h"
#include "model/session.h"
#include "assets.h"

int    home(struct http_request *);
void   home_error_handler(struct http_request *, int);

int 
home(struct http_request *req)
{
    if(req->method != HTTP_METHOD_GET)
    {
        return(KORE_RESULT_ERROR); //No methods besides GET exist on the home page
    }

    int return_code = (KORE_RESULT_OK);
    int err = 0;
    Session session = (Session) {
        .identifier = NULL,
        .user_identifier = 0
    };
    PartialContext context = {
        .session = &session  //TODO: fill from request cookie
    };

    if ((err = shared_http_find_session_from_request(req, &context.session)) != (SHARED_OK))
    {
        home_error_handler(req, err);
        return_code = (KORE_RESULT_OK);
        goto exit;
    }
    
    //a GET receives the home form and renders the page
    if((err = home_render(&context)) != (SHARED_OK))
    {
        home_error_handler(req, err);
        return_code = (KORE_RESULT_OK);
        goto exit;
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
        context.dst_context->string,
        strlen(context.dst_context->string));
exit:
    home_render_clean(&context);
    return return_code;    
}

void
home_error_handler(struct http_request *req, int errcode)
{
    shared_error_handler(req, errcode, "/");
}