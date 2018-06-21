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
        if((err = login_render(&context)) != (SHARED_ERROR_OK))
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

    if((err = login_parse_params(req, context.user)) != (SHARED_ERROR_OK))
    {
        login_error_handler(req, err, &context);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = login_try_login(context.user)) != (SHARED_ERROR_OK))
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

    return (SHARED_ERROR_OK);
}

uint32_t
login_try_login(User *input_user)
{
    uint32_t operation_result_code;     /* Error variable for the called functions. */
    uint32_t return_code;               /* Result variable for the result of this functoin. */
    User *database_user;

    database_user = user_find_by_username_or_email(input_user->email, 
        &operation_result_code);

    if(database_user == NULL)
    {
        switch(operation_result_code)
        {
            case (DATABASE_ENGINE_ERROR_NO_RESULTS):
                perror("login_try_login: Could not find a user with the " \
                    "corresponding email or username address.\n");
                return_code = operation_result_code;
                break;

            case (USER_CREATE_ERROR):
                perror("login_try_login: Could not handle user data.\n");
                return_code = operation_result_code;
                break; 

            case (DATABASE_ENGINE_ERROR_INITIALIZATION):
                perror("login_try_login: The database could not be reached.\n");
                return_code = operation_result_code;
                break;

            case (DATABASE_ENGINE_ERROR_QUERY_ERROR):
                perror("login_try_login: The database Encountered an error when executing the " \
                    "the query.\n");

            case (DATABASE_ENGINE_ERROR_RESULT_PARSE):
                perror("login_try_login: Could not parse the database's information.\n");
                return_code = operation_result_code;
                break; 

            default:
                perror("login_try_login: Could not execute query.\n");
                return_code = operation_result_code;
                break;
        } 

        return return_code;
    }

    bool login_attempt_result = false;
    if(login_check_bruteforce(database_user->identifier) != LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID)
    {
        if(login_validate_user_credentials(input_user, database_user) != 
            LOGIN_SUCCESSFULL_VALIDATION)
        {
            perror("login_try_login: Invalid user credentials have been detected.\n");
            return_code = LOGIN_ERROR_INVALLID_CREDENTIALS;
        }
        else
        {
            input_user = database_user;
            return_code = LOGIN_SUCCESSFULL_LOGIN;
            login_attempt_result = true;
        }
    }

    if(login_log_attempt(database_user->identifier, login_attempt_result) != 
        LOGIN_LOG_ATTEMPT_SUCCESS)
    {
        perror("login_try_login: Could not log login attempt.\n");
        return_code = LOGIN_ERROR_LOG_ERROR;
    }

    return return_code;
}

uint32_t
login_validate_user_credentials(const User *input_user, const User *database_user)
{
    uint32_t error;

    if(!(error = libscrypt_check(input_user->password, database_user->password)))
    {
        if(error < 0)
        {
            perror("login_validate_user_credentials: Encountered has error.\n");
            return (SHARED_ERROR_HASH_ERROR);
        }
        else if(error == 0)
        {
            return (LOGIN_ERROR_PASSWORD_INCORRECT);
        }
    }

    return (LOGIN_SUCCESSFULL_VALIDATION);
}

uint32_t
login_check_bruteforce(uint32_t user_identifier)
{
    if(login_attempt_amount_of_logins_in_five_minutes(user_identifier) >= 5)
    {
        return (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID);
    }

    return (LOGIN_BRUTEFORCE_CHECK_VALID);
}

uint32_t
login_log_attempt(uint32_t user_identifier, bool success)
{
    uint32_t error;
    uint32_t result;

    LoginAttempt *login_attempt = login_attempt_create(user_identifier, success, &error);
    if(error != (LOGIN_ATTEMPT_CREATE_SUCCESS))
    {
        perror("login_log_attempt: Could not create a login attempt structure.\n");
        result = (LOGIN_ATTEMPT_ERROR_CREATE);
        goto out;
    }

    if(login_attempt_insert(login_attempt) != (LOGIN_LOG_ATTEMPT_SUCCESS))
    {
        perror("login_log_attempt: Could not insert login attempt in the database.\n");
        result = (LOGIN_ATTEMPT_ERROR_INSERT);
        goto out;
    }

    result = (LOGIN_LOG_ATTEMPT_SUCCESS);

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
        case (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID):
                context->error_message = 
                "Your account has been locked. Please try again in several minutes.";
            break;
        case (LOGIN_ERROR_EMAIL_VALIDATOR_INVALID):
                context->error_message = 
                "Please use a correct email address. (e.g. test@example.com)";
            break;
        case (LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID):
                context->error_message = 
                "Please use a correct password (length: 8-32, may contain: a-zA-Z0-9._%+-@#$^&*() )";
            break;
        case (LOGIN_ERROR_EMAIL_INCORRECT):
        case (LOGIN_ERROR_PASSWORD_INCORRECT):
                context->error_message = 
                "Incorrect Email or Password.";
            break;
        case (LOGIN_ERROR_INVALLID_CREDENTIALS):
                context->error_message =
                "Incorrect Email or Password. (DEBUG: PASSWORD_INCORRECT)";
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
        if((err = login_render(context)) != (SHARED_ERROR_OK))
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