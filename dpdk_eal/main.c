#include <stdio.h>
#include <stdlib.h>
#include "eal_def.h"
#include "eal_log.h"
#include "eal_lcore.h"
#include "mem.h"
int main() 
{
    RTE_LOG(ERR, EAL, "Detected lcore %u as \n", 5);
    RTE_LOG(ERR, EAL, "Detected lcore  \n");
    // for(int i = 0; i < 16; ++i)
    // {
    //     int ret = eal_cpu_detected(i);
    //     if(0 == ret)
    //     {
    //         continue;
    //     }
    //     int id = eal_cpu_socket_id(i);
    //     int c = eal_cpu_core_id(i);
    //     printf(" cpu %d detected: %d socket_id: %d c: %d\n", i, ret, id, c);
    // }
    eal_cpu_init();
    printf_eal_config(&_eal_config);

    hugepage_info_init();
    printf_mem_conf(&mem_conf);
    return 0;
}
