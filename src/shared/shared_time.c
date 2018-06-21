#define _XOPEN_SOURCE 700 //must be defined for strptime from time.h
#include "shared/shared_time.h"

#include <time.h>

#include "shared/shared_error.h"

int shared_time_time_t_to_string(time_t *src, char *dest, const char *__restrict __format, int maxsize)
{
    struct tm *date_tm = localtime(src);
    if(NULL == date_tm)
    {
        return (SHARED_TIME_ERROR_PRINT);
    }
    if(strftime(dest, maxsize, __format, date_tm) == 0)
    {
        return (SHARED_TIME_ERROR_LOCALTIME);
    }

    return (SHARED_OK);
}