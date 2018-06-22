#ifndef SHARED_HTTP_H
#define SHARED_HTTP_H

#include <stdint.h>
#include <kore/http.h>

#include "model/user.h"
#include "model/session.h"

uint32_t
shared_http_get_user_from_request(struct http_request *req, User **user);

uint32_t
shared_http_get_session_from_request(struct http_request *req, Session **session);

#endif