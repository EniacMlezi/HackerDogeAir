#ifndef SESSION_H
#define SESSION_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct _Session
{
    uint32_t    session_identifier;
    uint32_t    user_identifier;
    tm          experiation_time;
} Session;

typedef struct
{
    Session session;
    LIST_ENTRY(_Session) list;
} SessionList;

Session *
session_create(
    uint32_t session_identifier,
    uint32_t user_identifier,
    tm experiation_time
    );

uint32_t
session_destroy(
    Session *session
    );

uint32_t
session_insert(
    const Session *session
    );

uint32_t
session_update(
    const Session *session
    );

uint32_t
session_delete(
    const Session *session
    );

Session *
session_find_by_session_identifier(
    uint32_t session_identifier
    );

Session *
session_find_by_user_identifier(
    uint32_t user_identifier
    );

SessionList *
session_list_create(
    void *source_location
    );

uint32_t
session_list_destroy(
    SessionList *session_list
    );

SessionList *
session_get_list(
    void
    );

#endif