#ifndef LOGIN_H
#define LOGIN_H

#include <stdint.h>
#include <stdbool.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "model/user.h"

#include "pages/shared/shared_user_render.h"

uint32_t   
login(
    struct http_request *req
    );

uint32_t    
login_parse_params(
    struct http_request *req, 
    User *user
    );

uint32_t    
login_try_login(
    User *input_user
    );

uint32_t    
login_check_bruteforce(
    uint32_t user_identifier
    );

uint32_t    
login_log_attempt(
    uint32_t user_identifier, 
    bool success
    );


void   
login_error_handler(
    struct http_request *rec, 
    uint32_t error, 
    UserContext *context
    );

uint32_t
login_validate_user_credentials(
    const User *input_user, 
    const User *database_user
    );

#endif