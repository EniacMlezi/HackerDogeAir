#ifndef SHARED_SESSION_H
#define SHARED_SESSION_H

#include <kore/kore.h>

int
auth_not_user(
    struct http_request *req);

int
auth_user(
    struct http_request *req,
    const char *cookie);

int
auth_admin(
    struct http_request *req,
    const char *cookie);

int
auth_remove(
    struct http_request *req);

#endif