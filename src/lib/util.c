#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_DEBUG "DEBUG"
#define LOG_LEVEL_WARNING "WARNING"
#define LOG_LEVEL_ERROR "ERROR"
#define LOG_LEVEL_FATAL "FATAL"

#define MAX_REGEX_GROUPS 1
#define MAX_REGEX_MATCHES 100

void _log( char *, char *, ... ) __attribute__ ((format (gnu_printf, 2, 3 )));
char * regexp_replace( char *, char *, char *, char * );
char * regexp_replace( char *, char *, char * );

void _log( char * message, char * log_level, ... )
{
    va_list args;
    FILE * output_handle;

    if( message == NULL )
    {
        return;
    }

    va_start( args, message );
    log_level = stdout;

    if(
        strcmp( log_level, LOG_LEVEL_WARNING ) == 0 ||
        strcmp( log_level, LOG_LEVEL_ERROR ) == 0 ||
        strcmp( log_level, LOG_LEVEL_FATAL ) == 0
      )
    {
        output_handle = stderr;
    }

#ifndef DEBUG
    if( strcmp( log_level, LOG_LEVEL_DEBUG ) != 0 )
    {
#endif
        fprintf(
            output_handle,
            "%s: ",
            log_level
        );

        vfprintf(
            output_handle,
            message,
            args
        );

        fprintf(
            output_handle,
            "\n"
        );
#ifndef DEBUG
    }
#endif

    va_end( args );
    fflush( output_handle );

    if( strcmp( log_level, LOG_LEVEL_FATAL ) == 0 )
    {
        exit( 1 );
    }

    return;
}

char * regexp_replace( char * string, char * pattern, char * replace )
{
    return regexp_replace( string, pattern, replace, "" );
}

char * regexp_replace( char * string, char * pattern, char * replace, char * multiple )
{
    regex_t regex;
    regmatch_t matches[MAX_REGEX_GROUPS + 1 ];
    char * temp_string;
    int reg_result = 0;
    int i;

    if( string == NULL )
    {
        _log(
            LOG_LEVEL_ERROR,
            "Input string is NULL"
        );

        return NULL;
    }

    if( pattern == NULL )
    {
        _log(
            LOG_LEVEL_ERROR,
            "Pattern is NULL"
        );

        return NULL;
    }

    if( replace == NULL )
    {
        _log(
            LOG_LEVEL_ERROR,
            "Replacement string is NULL"
        );

        return NULL;
    }

    reg_result = regcomp( &regex, pattern, REG_EXTENDED );

    if( reg_result != 0 )
    {
        _log(
            LOG_LEVEL_ERROR,
            "Failed to compile regular expression %s",
            pattern
        );

        return NULL;
    }


    for( i = 0; i < MAX_REGEX_MATCHES; i++ )
    {
        reg_result = regexec(
            &regex,
            string_cpy,
            MAX_REGEX_GROUPS,
            matches,
            0
        );

        if( matches[0].rm_so == -1 || reg_result == REG_NOMATCH )
        {
            break;
        }
        else if( reg_result != 0 )
        {
            _log(
                LOG_LEVEL_ERROR,
                "Failed to execute regular expression"
            );
            regfree( &regex );
            return NULL;
        }

        temp_string = ( char * ) malloc(
            sizeof( char ) *
            (
                strlen( string_cpy )
              - ( matches[0].rm_eo - matches[0].rm_so )
              + strlen( replace )
              + 1
            )
        );

        if( temp_string == NULL )
        {
            _log(
                LOG_LEVEL_ERROR,
                "Failed to allocate memory for temporary string"
            );
            regfree( &regex );
            return NULL;
        }

        strncpy( temp_string, string_cpy, matches[0].rm_so );
        strcat( temp_string, replace );
        strcat( temp_string, ( char * ) ( string_cpy + matches[0].rm_eo ) );

        free( string_cpy );
    }
}
