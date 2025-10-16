#ifndef _lib_LOG_H_
#define _lib_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>

#define __time_fmt "%04d-%02d-%02d-%02d:%02d:%02d.%03ld"

#define __time_args(tm_info) \
    tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday, \
    tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, \
    ts.tv_nsec / 1000000
    
#define FILE_LOG(file,fmt, ...) \
    do { \
        struct timespec ts; struct tm *tm_info;\
        clock_gettime(CLOCK_REALTIME, &ts);\
        tm_info = localtime(&ts.tv_sec);\
        fprintf(file, __time_fmt "%s:%s:%d: " fmt "\n", __time_args(tm_info), __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0) 

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

#define ERROR_LOG(fmt, ...)     FILE_LOG(stderr, RED "error: " RESET fmt, ##__VA_ARGS__)
#if NDEBUG
#define DEBUG_LOG(fmt, ...)     ((void)0)
#else
#define DEBUG_LOG(fmt, ...)     FILE_LOG(stdout, YELLOW "debug: " RESET fmt, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
