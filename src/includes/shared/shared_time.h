#ifndef SHARED_TIME_H
#define SHARED_TIME_H

#include <stdint.h>
#include <time.h>

#define SHARED_TIME_ERROR_LOCALTIME 1101
#define SHARED_TIME_ERROR_PRINT 1102

char *
shared_time_tm_to_database_string(
    struct tm time,
    char *destination_string,
    uint32_t *error
    );

uint32_t
shared_time_database_string_to_tm(
    const char *source_location,
    struct tm *destination
    );

uint32_t
shared_time_user_input_string_to_tm(
    const char *source_location, 
    struct tm *destination
    );

#endif