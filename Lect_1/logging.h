#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


void logging_info(const char *file, int lineNo, const char *func, const char *message, ...);
void logging_warn(const char *file, int lineNo, const char *func, const char *message, ...);
void logging_fatal(const char *file, int lineNo, const char *func, const char *message, ...);
void logging_error(const char *file, int lineNo, const char *func, const char *message, ...);
void logging_debug(const char *file, int lineNo, const char *func, const char *message, ...);
void logging_notset(const char *file, int lineNo, const char *func, const char *message, ...);

#define info(message, ...) logging_info( __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__)
#define warn(message, ...) logging_warn( __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__)
#define fatal(message, ...) logging_fatal( __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__)
#define error(message, ...) logging_error( __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__)
#define debug(message, ...) logging_debug( __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__)
#define notset(message, ...) logging_notset( __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__)

#endif // LOGGING_H_INCLUDED