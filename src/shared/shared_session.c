#include "shared/shared_session.h"

#include <time.h>
#include <kore/kore.h>

#include "shared/shared_error.h"
#include "model/session.h"
#include "model/user.h"
#include "model/role.h"

int
auth_user(struct http_request *req, const char *cookie)
{
    int return_code = (KORE_RESULT_OK);

    uint32_t error = 0;
    if (cookie == NULL)
    {
        return (KORE_RESULT_ERROR);
    }

    //find session
    Session *session = session_find_by_session_identifier(cookie, &error);
    if(error != (SHARED_OK) && error != (DATABASE_ENGINE_ERROR_NO_RESULTS))
    {
        kore_log(LOG_ERR, "auth_user: Failed to get session from database. error: %d", error);
        return (KORE_RESULT_ERROR);
    }
    else if (error == (DATABASE_ENGINE_ERROR_NO_RESULTS))
    { 
        return (KORE_RESULT_ERROR);
    }
    //check expiration time
    time_t epoch_expiration_time = mktime(&session->expiration_time);
    time_t epoch_current_time = time(NULL);
    if(epoch_current_time > epoch_expiration_time)
    {
        auth_remove(req);
        session_destroy(session);
        return (KORE_RESULT_ERROR);
    }

    //update expiration time
    time_t epoch_new_expiration_time = epoch_current_time + 60 * 60;
    struct tm *new_expiration_time = localtime(&epoch_new_expiration_time);
    session->expiration_time = *new_expiration_time;
    if((error = session_update(session)) != (SHARED_OK))
    {
        kore_log(LOG_ERR, "auth_user: failed to update session");
        session_destroy(session);
        return (KORE_RESULT_ERROR);
    }
    //update cookie expiration time
    http_response_cookie(req, "session", cookie, "/", epoch_new_expiration_time, 60*60, NULL);
    session_destroy(session);
    return (KORE_RESULT_OK);
}

int
auth_admin(struct http_request *req, const char *cookie)
{
    int return_code = (KORE_RESULT_OK);

    uint32_t error = 0;
    if (cookie == NULL)
    {
        return (KORE_RESULT_ERROR);
    }

    //find session
    Session *session = session_find_by_session_identifier(cookie, &error);
    if(error != (SHARED_OK) && error != (DATABASE_ENGINE_ERROR_NO_RESULTS))
    {
        kore_log(LOG_ERR, "auth_admin: Failed to get session from database. error: %d", error);
        return (KORE_RESULT_ERROR);
    }
    else if (error == (DATABASE_ENGINE_ERROR_NO_RESULTS))
    { 
        return (KORE_RESULT_ERROR);
    }
    //check expiration time
    time_t epoch_expiration_time = mktime(&session->expiration_time);
    time_t epoch_current_time = time(NULL);
    if(epoch_current_time > epoch_expiration_time)
    {
        auth_remove(req);
        session_destroy(session);
        return (KORE_RESULT_ERROR);
    }

    //check admin
    User *user = user_find_by_identifier(session->user_identifier, &error);
    if(error != (SHARED_OK) || user == NULL)
    {
        kore_log(LOG_ERR, "auth_admin: Failed to get user from database.");
        session_destroy(session);
        return (KORE_RESULT_ERROR);
    }
    else if(user->role != ADMIN)
    {
        kore_log(LOG_WARNING, "auth_admin: A non-privileged user tried to access /admin/*");
        return (KORE_RESULT_ERROR);
    }

    //update expiration time
    time_t epoch_new_expiration_time = epoch_current_time + 60 * 60;
    struct tm *new_expiration_time = localtime(&epoch_new_expiration_time);
    session->expiration_time = *new_expiration_time;
    if((error = session_update(session)) != (SHARED_OK))
    {
        kore_log(LOG_ERR, "auth_admin: failed to update session");
        session_destroy(session);
        return (KORE_RESULT_ERROR);
    }
    //update cookie expiration time
    http_response_cookie(req, "session", cookie, "/", epoch_new_expiration_time, 60*60, NULL);
    session_destroy(session);
    return (KORE_RESULT_OK);
}

int
auth_remove(struct http_request *req)
{
    uint32_t error = 0;
    char *session_identifier;
    http_populate_cookies(req);
    if (http_request_cookie(req, "session", &session_identifier) != (KORE_RESULT_OK)) 
    {
        kore_log(LOG_DEBUG, "COOKIE not found. can't log out");
        return (SHARED_OK); //nothing to do
    }

    http_response_cookie(req, "session", "", "/", 0, 0, NULL);
    //remove from db
    Session session = {
        .identifier = session_identifier
    };
    if((error = session_delete(&session)) != (SHARED_OK))
    {
        kore_log(LOG_ERR, "auth_remove: failed to remove session");
        return error;
    }

    return (SHARED_OK);
}
