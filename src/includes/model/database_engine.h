#ifndef DATA_BASE_ENGHINE_H
#define DATA_BASE_ENGHINE_H

#define DATABASE_NAME "DogeAir"

/*
#define DATABASE_ENGINE_OK                              0
#define DATABASE_ENGINE_ERROR_NO_RESULTS                1
#define DATABASE_ENGINE_ERROR_INITIALIZATION            2
#define DATABASE_ENGINE_ERROR_QUERY_ERROR               3
#define DATABASE_ENGINE_ERROR_INVALLID_RESULT           4
#define DATABASE_ENGINE_ERROR_RESULT_PARSE              5
*/

#include <stdint.h>
#include <stdarg.h>

uint32_t
database_engine_execute_write(
    const char *sql_query,
    uint32_t count,
    ...
    );

void *
database_engine_execute_read(
    const char *sql_query,
    void *(*model_build_from_query)(void *source_location, uint32_t *error),
    uint32_t *error,
    uint32_t count,
    ...
    );

#endif