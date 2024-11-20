#ifndef _EAL_LOG_H_
#define _EAL_LOG_H_
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define  ERR "EAL ERR %s:%u " 
#define  CRIT "EAL CRIT %s:%u " 
#define  DEBUG "EAL DEBUG %s:%u " 
#define  NOTICE "EAL NOTICE %s:%u " 
#define  WARNING "EAL WARNING %s:%u " 


#define  EAL ""
#define RTE_LOG(l,v,fmt, ...) fprintf(stderr, l fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define rte_panic(fmt, ...) __rte_panic(__func__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

// 函数实现
static inline void __rte_panic(const char *funcname, const char *format, ...) 
{
    va_list args;
    fprintf(stderr, "Panic in %s: ", funcname);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

// 示例宏

#ifdef __cplusplus
}
#endif

#endif