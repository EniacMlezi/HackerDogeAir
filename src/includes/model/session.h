#ifndef SESSION_H
#define SESSION_H

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

typedef struct
{
    uint32_t    session_identifier;
    uint32_t    user_identifier;
    struct tm   experiation_time;
} Session;

typedef struct SessionCollection
{
    Session session;
    TAILQ_ENTRY(SessionCollection) session_collection;
} SessionCollection;

Session *
session_create(
    uint32_t session_identifier,
    uint32_t user_identifier,
    struct tm experiation_time,
    uint32_t *error
    );

void *
session_create_from_query(
    void *source_location, 
    uint32_t *error
    );

void
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

#endif