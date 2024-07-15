#ifndef _EAL_LCORE_H_
#define _EAL_LCORE_H_


#ifdef __cplusplus
extern "C" {
#endif

int eal_cpu_detected(unsigned lcore_id);

unsigned eal_cpu_socket_id(unsigned lcore_id);

unsigned eal_cpu_core_id(unsigned lcore_id);

#ifdef __cplusplus
}
#endif

#endif