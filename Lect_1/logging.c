#include "logging.h"
#define _level_NOTSET 0
#define _level_DEBUG 10
#define _level_INFO 20
#define _level_WARN 30
#define _level_ERROR 40
#define _level_FATAL 50
#define logBufferLen 4096

int log_output_level;

static char logBuffer[logBufferLen];

#define lvl2str(lvl) (lvl > 25 ? (lvl == _level_FATAL ? "\e[1;31m[FATAL]\e[m " : (lvl == _level_WARN ? "\e[33m[WARN]\e[m  " : "\e[1;31m[ERROR]\e[m ")) \
                               : (lvl == _level_DEBUG ? "\e[0;36m[DEBUG]\e[m " : (lvl == _level_INFO ? "\e[33m[INFO]\e[m  " : "\e[34m[NOTSET]\e[m")))
static void _vlog(short level, const char *file, int lineNo, const char *func, const char *message, va_list args)
{

    size_t flen = snprintf(logBuffer, logBufferLen, "%s %s % 4d %-15s %d %lu │ ", lvl2str(level), file, lineNo, func, 0xff & getpid(), 0xff & pthread_self()->__sig);
    if (message != NULL)
    {
        flen += vsnprintf(logBuffer + flen, logBufferLen - flen, message, args);
    }
    if (flen < logBufferLen)
        flen += snprintf(logBuffer + flen, logBufferLen - flen, "\n");
    fprintf(stdout,"%s", logBuffer);
}
void logging_info(const char *file, int lineNo, const char *func, const char *message, ...)
{
    if (_level_INFO < log_output_level)
        return;
    va_list args;
    va_start(args, message);
    _vlog(_level_INFO, file, lineNo, func, message, args);
    va_end(args);
}

void logging_warn(const char *file, int lineNo, const char *func, const char *message, ...)
{
    if (_level_WARN < log_output_level)
        return;
    va_list args;
    va_start(args, message);
    _vlog(_level_WARN, file, lineNo, func, message, args);
    va_end(args);
}

void logging_fatal(const char *file, int lineNo, const char *func, const char *message, ...)
{
    if (_level_FATAL < log_output_level)
        return;
    va_list args;
    va_start(args, message);
    _vlog(_level_FATAL, file, lineNo, func, message, args);
    va_end(args);
}

void logging_error(const char *file, int lineNo, const char *func, const char *message, ...)
{
    if (_level_ERROR < log_output_level)
        return;
    va_list args;
    va_start(args, message);
    _vlog(_level_ERROR, file, lineNo, func, message, args);
    va_end(args);
}

void logging_debug(const char *file, int lineNo, const char *func, const char *message, ...)
{
    if (_level_DEBUG < log_output_level)
        return;
    va_list args;
    va_start(args, message);
    _vlog(_level_DEBUG, file, lineNo, func, message, args);
    va_end(args);
}

void logging_notset(const char *file, int lineNo, const char *func, const char *message, ...)
{
    if (_level_NOTSET < log_output_level)
        return;
    va_list args;
    va_start(args, message);
    _vlog(_level_NOTSET, file, lineNo, func, message, args);
    va_end(args);
}
