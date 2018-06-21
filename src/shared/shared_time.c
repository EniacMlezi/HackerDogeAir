#define _XOPEN_SOURCE 700 //must be defined for strptime from time.h
#include "shared/shared_time.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "shared/shared_error.h"

char *
shared_time_tm_to_database_string(struct tm time, char *destination_string, uint32_t *error)
{
   if(strftime(destination_string, strlen(destination_string), "%Y-%m-%d %H:%M:%S", &time) !=
    strlen(destination_string))
   {
        perror("shared_time_tm_to_database_string: Encountered a conversion error.\n");
        *error = (SHARED_TIME_CONVERSION_ERROR);
        return NULL;
   }

   *error = (SHARED_ERROR_OK);
   return destination_string;
}

uint32_t
shared_time_database_string_to_tm(const char *source_location, struct tm *destination)
{
    /* This function return both a nullptr in the case of an error and when the complete 
    source_location is processed and is thus not handable. */
    strptime(source_location, "%Y-%m-%d %H:%M:%S", destination);
    return (SHARED_TIME_CONVERSION_OK);
}

uint32_t
shared_time_user_input_string_to_tm(const char *source_location, struct tm *destination)
{
    /* This function return both a nullptr in the case of an error and when the complete 
    source_location is processed and is thus not handable. */
    strptime(source_location, "%d-%m-%Y", destination);
    return (SHARED_TIME_CONVERSION_OK);
}