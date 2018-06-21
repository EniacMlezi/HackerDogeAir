#ifndef DATA_BASE_ENGHINE_H
#define DATA_BASE_ENGHINE_H

#define DATABASE_NAME "DogeAir"

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


void 
database_engine_log_error(
    const char *prefix, 
    uint32_t error);

#endif