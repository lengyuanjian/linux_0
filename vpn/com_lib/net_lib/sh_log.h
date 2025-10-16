#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

class log_event
{
    void write_msg(const char * type, FILE *stream, const char *format, va_list args)
    {
        struct timespec ts;
        struct tm *tm_info;
        clock_gettime(CLOCK_REALTIME, &ts);
        tm_info = localtime(&ts.tv_sec);
        fprintf(stream, "%04d-%02d-%02d-%02d:%02d:%02d.%03ld",
                tm_info->tm_year + 1900,
                 tm_info->tm_mon + 1,
                 tm_info->tm_mday,
                 tm_info->tm_hour,
                 tm_info->tm_min,
                 tm_info->tm_sec,
                 ts.tv_nsec / 1000000);
        fprintf(stream, "-%s-", type);
        vfprintf(stream, format, args);
        fprintf(stream, "\n");
    }
public:
    virtual ~log_event(){}
    
    // 2025-03-19-16:02:22-331180-DEBUG-main.cpp:123-msg
    
    virtual void on_debug(FILE *stream, const char * fmt,...)
    {
        va_list args;
        va_start(args, fmt);
        write_msg("DEBUG", stream, fmt, args);
        va_end(args);
    }
    virtual void on_info(FILE *stream, const char * fmt,...)
    {
        va_list args;
        va_start(args, fmt);
        write_msg("INFO", stream, fmt, args);
        va_end(args);
    }   
    virtual void on_error(FILE *stream, const char * fmt,...)
    {
        va_list args;
        va_start(args, fmt);
        write_msg("ERROR", stream, fmt, args);
        va_end(args);
    }
};

