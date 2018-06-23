#include <kore/kore.h>
#include <kore/http.h>

#include "shared/shared_session.h"
#include "pages/home/home.h"

#include "assets.h"

uint32_t logout(struct http_request *req);

uint32_t 
logout(struct http_request *req)
{
    auth_remove(req);
    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, 
            asset_logout_success_html,
            asset_len_logout_success_html);

    return (KORE_RESULT_OK);
}