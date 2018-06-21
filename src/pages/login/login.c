#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "pages/login/login_render.h"
#include "model/user.h"
#include "model/login_attempt.h"
#include "assets.h"
#include "pages/login.h"

uint32_t 
login(struct http_request *req)
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

    if(req->method == HTTP_METHOD_GET)
    {   
        //a GET receives the login form
        if((err = login_render(&context)) != (SHARED_OK))
        {
            login_error_handler(req, err, &context);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context.partial_context.dst_context->string, 
            strlen(context.partial_context.dst_context->string));

        login_render_clean(&context);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only serve GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    if((err = login_parse_params(req, context.user)) != (SHARED_OK))
    {
        login_error_handler(req, err, &context);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = login_try_login(context.user)) != (SHARED_OK))
    {   //when not logged in correctly, notify user.
        login_error_handler(req, err, &context);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit  
    }

    http_response_header(req, "content-type", "text/html");
    http_response(req, HTTP_STATUS_OK, asset_login_success_html, asset_len_login_success_html);

    return (KORE_RESULT_OK);
}

uint32_t
login_parse_params(struct http_request *req, User *user)
{
    http_populate_post(req);
    if(!http_argument_get_string(req, "email", &(user->email)))
    {
        return (LOGIN_ERROR_EMAIL_VALIDATOR_INVALID); 
    }
    if(!http_argument_get_string(req, "password", &(user->password)))
    {
        return (LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID); 
    }

    return (SHARED_OK);
}

uint32_t
login_try_login(User *input_user)
{
    uint32_t error;         /* Error variable for the called functions. */
    uint32_t return_code = (SHARED_OK);   /* Result variable for the result of this functoin. */
    User *database_user;

    database_user = user_find_by_username_or_email(input_user->email, &error);
    if(database_user == NULL)
    {
        if(error == (DATABASE_ENGINE_ERROR_NO_RESULTS))
        {
            error = (LOGIN_ERROR_EMAIL_INCORRECT);
        }
        return error;
    }

    bool login_attempt_result = false;
    if((error = login_check_bruteforce(database_user->identifier)) != (SHARED_OK))
    {
        if(error != (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID))
        {
            return error;
        }
        return_code = (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID);
    }
    else
    {
        if((error = login_validate_password(database_user->password, input_user->password)) != 
        (SHARED_OK))
        {
            if(error != (LOGIN_ERROR_PASSWORD_INCORRECT))
            {
                return error;
            }
            return_code = (LOGIN_ERROR_PASSWORD_INCORRECT);
        }
        else
        {
            input_user = database_user;
            login_attempt_result = true;
        }
    }

    if(login_log_attempt(database_user->identifier, login_attempt_result) != 
        (SHARED_OK))
    {
        return_code = (LOGIN_ERROR_LOG_ATTEMPT_ERROR);
    }

    user_destroy(database_user);

    return return_code;
}

uint32_t
login_validate_password(char *password1, char *password2)
{
    uint32_t error;

    if(!(error = libscrypt_check(password1, password2)))
    {
        if(error < 0)
        {
            return (SHARED_ERROR_HASH_ERROR);
        }
        else if(error == 0)
        {
            return (LOGIN_ERROR_PASSWORD_INCORRECT);
        }
    }

    return (SHARED_OK);
}

uint32_t
login_check_bruteforce(uint32_t user_identifier)
{
    uint32_t error;
    uint32_t result;
    result = login_attempt_amount_of_logins_in_five_minutes(user_identifier, &error);
    if(error != (SHARED_OK) && error != (DATABASE_ENGINE_ERROR_NO_RESULTS))
    {
        return error;
    }
    if(result >= 5)
    {
        return (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID);
    }

    return (SHARED_OK);
}

uint32_t
login_log_attempt(uint32_t user_identifier, bool success)
{
    uint32_t error;
    uint32_t result;

    struct tm login_time = (struct tm) {
        .tm_year     = 0,
        .tm_mon      = 0,
        .tm_mday     = 0,
        .tm_hour     = 0,
        .tm_min      = 0,
        .tm_sec      = 0,  
        .tm_wday     = 0,
        .tm_yday     = 0
    };

    LoginAttempt *login_attempt = login_attempt_create(user_identifier, success, login_time, 
        &error);

    if(error != (SHARED_OK))
    {
        result = (LOGIN_ERROR_LOG_ATTEMPT_ERROR);
        goto out;
    }

    if(login_attempt_insert(login_attempt) != (SHARED_OK))
    {
        result = (LOGIN_ERROR_LOG_ATTEMPT_ERROR);
        goto out;
    }

    result = (SHARED_OK);

    out:
    login_attempt_destroy(login_attempt);
    return result; 
}

void
login_error_handler(struct http_request *req, uint32_t errcode, UserContext *context)
{
    bool handled = true;
    int err = 0;
    switch (errcode)
    {
        case (LOGIN_ERROR_EMAIL_VALIDATOR_INVALID):
                context->error_message = 
                "Please use a correct email address. (e.g. test@example.com)";
            break;
        case (LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID):
                context->error_message = 
                "Please use a correct password (length: 8-32, may contain: a-zA-Z0-9._%+-@#$^&*() )";
            break;
        case (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID):
                context->error_message = 
                "Your account has been locked. Please try again in several minutes.";
            break;
        case (LOGIN_ERROR_EMAIL_INCORRECT):
        case (LOGIN_ERROR_PASSWORD_INCORRECT):
                context->error_message = 
                "Incorrect Email or Password.";
            break;
        case (LOGIN_ERROR_LOG_ATTEMPT_ERROR):
            shared_error_response(req, HTTP_STATUS_INTERNAL_ERROR,
                "Internal Server error: Failed to log attempt", "/login", 5);
            break;

        default: 
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode, "/login");
    }
    else
    {
        if((err = login_render(context)) != (SHARED_OK))
        {
            login_error_handler(req, err, context);
        }

        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, 
            context->partial_context.dst_context->string, 
            strlen(context->partial_context.dst_context->string));

        login_render_clean(context);
    }
}