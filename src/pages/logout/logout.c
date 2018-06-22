#include <kore/kore.h>

#include "shared/shared_session.h"
#include "pages/home/home.h"

uint32_t logout(struct http_request *req);

uint32_t 
logout(struct http_request *req)
{
    auth_remove(req);
    return home(req);
}