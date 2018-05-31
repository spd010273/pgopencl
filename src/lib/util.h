#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_DEBUG "DEBUG"
#define LOG_LEVEL_WARNING "WARNING"
#define LOG_LEVEL_ERROR "ERROR"
#define LOG_LEVEL_FATAL "FATAL"

#define MAX_REGEX_GROUPS 1
#define MAX_REGEX_MATCHES 100

void _log( char *, char *, ... ) __attribute__ ((format (gnu_printf, 2, 3 )));
char * regexp_replace( char *, char *, char * );
char * regexp_replace( char *, char *, char *, char * );
