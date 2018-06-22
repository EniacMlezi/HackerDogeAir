#ifndef SHARED_TIME_H
#define SHARED_TIME_H

#define SHARED_TIME_TM_TO_DATABASE_FORMAT_STRING_SIZE 17

#include <stdint.h>
#include <time.h>

char *
shared_time_tm_to_database_string(
    const struct tm *time,
    char *destination_string,
    uintmax_t size, 
    uint32_t *error
    );

uint32_t
shared_time_database_string_to_tm(
    const char *source_location,
    struct tm *destination
    );

uint32_t
shared_time_tm_to_string(
    const struct tm *source,
    char *destination,
    uintmax_t size,
    const char *__restrict __format
    );

uint32_t
shared_time_string_to_tm(
    const char *source,
    struct tm *destination,
    const char *__restrict __format
    );

uint32_t
shared_time_tm_to_epoch(
    struct tm *time, 
    time_t *epoch
    );

#endif