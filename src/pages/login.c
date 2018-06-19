#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <libscrypt.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include "shared/shared_error.h"
#include "model/user.h"
#include "model/login_attempt.h"
#include "assets.h"
#include "pages/login.h"

uint32_t 
login(struct http_request *req)
{
    int err;
    User user = {0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL};

    if(req->method == HTTP_METHOD_GET)
    {   //a GET receives the login form
        http_response_header(req, "content-type", "text/html");
        http_response(req, HTTP_STATUS_OK, asset_login_html, asset_len_login_html);
        return (KORE_RESULT_OK);
    }
    else if(req->method != HTTP_METHOD_POST)
    {   //only serve GET and POST requests
        return (KORE_RESULT_ERROR);
    }

    if((err = login_parse_params(req, &user)) != (SHARED_ERROR_OK))
    {
        login_error_handler(req, err);
        return (KORE_RESULT_OK);    //KORE_OK for graceful exit
    }

    if((err = login_try_login(&user)) != (SHARED_ERROR_OK))
    {   //when not logged in correctly, notify user.
        login_error_handler(req, err);
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

    database_user = user_find_by_username_and_email(input_user->email, input_user->username, 
        &operation_result_code);

    if(operation_result_code != USER_CREATE_SUCCESS)
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
            case (DATABASE_ENGINE_ERROR_QUERY_ERROR):
            case (DATABASE_ENGINE_ERROR_RESULT_PARSE):
            default:
                perror("login_try_login: Could not execute query.\n");
                return_code = operation_result_code;
            break;
        } 

        return return_code;
    }

    bool login_attempt_result;
    if(login_check_bruteforce(database_user->identifier) != LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID)
    {
        if(login_validate_user_credentials(input_user, database_user) != 
            LOGIN_SUCCESSFULL_VALIDATION)
        {
            perror("login_try_login: Invalid user credentials have been detected.\n");
            return_code = LOGIN_ERROR_INVALLID_CREDENTIALS;
            login_attempt_result = false;
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
    if(login_attempt_amount_of_logins_in_x_minutes(user_identifier, 5) >= 5)
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
login_error_handler(struct http_request *req, uint32_t errcode)
{
    bool handled = true;
    switch (errcode)
    {
        case (LOGIN_ERROR_BRUTEFORCE_CHECK_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Your account has been locked. Please try again in several minutes.");
            break;
        case (LOGIN_ERROR_EMAIL_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST,
                "Email format incorrect. Validator failed.");
            break;
        case (LOGIN_ERROR_PASSWORD_VALIDATOR_INVALID):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Password format incorrect. Validator failed.");
            break;
        case (LOGIN_ERROR_EMAIL_INCORRECT):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST,
                "Incorrect Email or Password. (DEBUG: EMAIL_INCORRECT)");
            break;
        case (LOGIN_ERROR_PASSWORD_INCORRECT):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Incorrect Email or Password. (DEBUG: PASSWORD_INCORRECT)");
            break;
        case (LOGIN_ERROR_INVALLID_CREDENTIALS):
            shared_error_response(req, HTTP_STATUS_BAD_REQUEST, 
                "Incorrect Email or Password. (DEBUG: PASSWORD_INCORRECT)");
            break;

        default: 
            handled = false;
    }

    if(!handled)
    {
        shared_error_handler(req, errcode);
    }
}