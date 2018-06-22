#define _XOPEN_SOURCE 700 //must be defined for strptime from time.h
#include "shared/shared_time.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "shared/shared_error.h"

char *
shared_time_tm_to_database_string(const struct tm *time, char *destination_string, uintmax_t size, 
    uint32_t *error)
{
    if(strftime(destination_string, size, "%Y-%m-%d %H:%M:%S", time) == 0)
    {
      *error = (SHARED_ERROR_TIME_CONVERSION);
      return NULL;
    }

    *error = (SHARED_OK);
    return destination_string;
}

uint32_t
shared_time_database_string_to_tm(const char *source_location, struct tm *destination)
{
    if(strptime(source_location, "%Y-%m-%d %H:%M:%S", destination) == NULL)
    {
        return (SHARED_ERROR_TIME_CONVERSION);
    }
    return (SHARED_OK);
}

uint32_t
shared_time_tm_to_string(const struct tm *source, char *destination, uintmax_t size, 
    const char *__restrict __format)
{
    if(strftime(destination, size, __format, source) == 0)
    {
      return (SHARED_ERROR_TIME_CONVERSION);
    }

    return (SHARED_OK);
}

uint32_t
shared_time_string_to_tm(const char *source, struct tm *destination, 
    const char *__restrict __format)
{
    if(strptime(source, __format, destination) == NULL)
    {
        return (SHARED_ERROR_TIME_CONVERSION);
    }

    return (SHARED_OK);
}

uint32_t
shared_time_tm_to_epoch(struct tm *time, time_t *epoch)
{
    *epoch = mktime(time);
    if(*epoch == -1)
    {
        return (SHARED_ERROR_TIME_CONVERSION);
    }
    return (SHARED_OK);
}