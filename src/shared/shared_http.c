#include "shared/shared_http.h"

#include <stdint.h>
#include <kore/kore.h>
#include <kore/http.h>
#include "shared/shared_error.h"
#include "model/user.h"
#include "model/session.h"

uint32_t
shared_http_get_user_from_request(struct http_request *req, User **user)
{
    uint32_t error = 0;
    char *session_identifier;
    http_populate_cookies(req);
    if (http_request_cookie(req, "session", &session_identifier) != (KORE_RESULT_OK)) 
    {
        kore_log(LOG_ERR, "get_user_from_request: Session cookie not found");
        return (SHARED_ERROR_COOKIE_NOT_FOUND);
    }

    *user = user_find_by_session_identifier(session_identifier, &error);
    return error;
}

uint32_t
shared_http_get_session_from_request(struct http_request *req, Session **session)
{
    uint32_t error = 0;
    char *session_identifier;
    http_populate_cookies(req);
    if (http_request_cookie(req, "session", &session_identifier) != (KORE_RESULT_OK)) 
    {
        kore_log(LOG_ERR, "get_user_from_request: Session cookie not found");
        return (SHARED_ERROR_COOKIE_NOT_FOUND);
    }
    *session = session_find_by_session_identifier(session_identifier, &error);
    return error;
}

uint32_t
shared_http_find_session_from_request(struct http_request *req, Session **session)
{
    uint32_t error = 0;
    char *session_identifier;
    http_populate_cookies(req);
    if (http_request_cookie(req, "session", &session_identifier) != (KORE_RESULT_OK)) 
    {
        return (SHARED_OK);
    }
    kore_log(LOG_ERR, "shared_http: session_identifier %s", session_identifier);
    if (session_identifier == NULL) {
        return (SHARED_OK);
    }
    *session = session_find_by_session_identifier(session_identifier, &error);
    return error;
}