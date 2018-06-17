#ifndef SESSION_H
#define SESSION_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct
{
    uint32_t    session_identifier;
    uint32_t    user_identifier;
    tm          experiation_time;
} Session;

typedef struct SessionCollection
{
    Session session;
    LIST_ENTRY(SessionCollection) session_collection;
} SessionCollection;

Session *
session_create(
    uint32_t session_identifier,
    uint32_t user_identifier,
    tm experiation_time,
    uint32_t error
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
    Session *session
    );

Session *
session_find_by_session_identifier(
    uint32_t session_identifier,
    uint32_t *error
    );

Session *
session_find_by_user_identifier(
    uint32_t user_identifier,
    uint32_t *error
    );

SessionCollection *
session_collection_create(
    void *source_location,
    uint32_t *error
    );

uint32_t
session_colection_destroy(
    SessionCollection *session_list
    );

SessionCollection *
session_get_collection(
    uint32_t *error
    );

#endif