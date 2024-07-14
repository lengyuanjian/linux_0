#ifndef _SH_LOG_H_
#define _SH_LOG_H_
#include <stdio.h>
#include <stdlib.h>

#define  ERR "EAL ERR %s:%u " 
#define  DEBUG "EAL DEBUG %s:%u " 
#define  EAL ""
#define RTE_LOG(l,v,fmt, ...) fprintf(stderr, l fmt, __FILE__, __LINE__, ##__VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif
 

#ifdef __cplusplus
}
#endif

#endif