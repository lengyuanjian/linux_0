#include "eal_def.h"
#include "eal_lcore.h"
 #include <stdlib.h>


struct eal_config _eal_config;

static int
socket_id_cmp(const void *a, const void *b)
{
	const int *lcore_id_a = a;
	const int *lcore_id_b = b;

	if (*lcore_id_a < *lcore_id_b)
		return -1;
	if (*lcore_id_a > *lcore_id_b)
		return 1;
	return 0;
}

int
eal_cpu_init(void)
{
    _eal_config.lcore_count = 0;
    int lcore_to_socket_id[RTE_MAX_LCORE];
    for(int lcore_id = 0; lcore_id < RTE_MAX_LCORE; ++lcore_id)
    {
        struct lcore_config * p_core = &_eal_config.lcore_config[lcore_id];

        unsigned socket_id = eal_cpu_socket_id(lcore_id);
        p_core->socket_id = socket_id;
        
        p_core->detected = eal_cpu_detected(lcore_id);
		if (p_core->detected == 0) 
        {
            continue;
        }
        
        unsigned core_id = eal_cpu_core_id(lcore_id);
        p_core->core_id = core_id;
        
        lcore_to_socket_id[lcore_id] = socket_id;

        p_core->core_index = _eal_config.lcore_count++;
    }
    if( _eal_config.lcore_count == 0)
    {
        return -1;
    }
	qsort(lcore_to_socket_id, _eal_config.lcore_count,
			sizeof(lcore_to_socket_id[0]), socket_id_cmp);
    
    unsigned prev_socket_id = (unsigned)-1;
    _eal_config.numa_node_count = 0;
    for(int lcore_id = 0; lcore_id < _eal_config.lcore_count; ++lcore_id) 
    {
		unsigned socket_id = lcore_to_socket_id[lcore_id];
		if (socket_id != prev_socket_id)
        {
			_eal_config.numa_nodes[_eal_config.numa_node_count++] = socket_id;
        }
		prev_socket_id = socket_id;
	}
    
    return 0;
}

unsigned int rte_socket_count()
{
    return _eal_config.numa_node_count;
}

int rte_socket_id_by_idx(unsigned int idx)
{
    if (idx >= _eal_config.numa_node_count) 
    {
		return -1;
	}
	return _eal_config.numa_nodes[idx];
}

#include <stdio.h>
void printf_eal_config(struct eal_config * p_config)
{
    if (p_config == NULL) {
        printf("Invalid eal_config pointer.\n");
        return;
    }

    printf("EAL Configuration:\n");
    printf("Number of detected NUMA nodes: %u\n", p_config->numa_node_count);
    printf("Detected NUMA nodes: ");
    for (unsigned i = 0; i < p_config->numa_node_count; ++i) {
        printf("%u ", p_config->numa_nodes[i]);
    }
    printf("\n");

    printf("Number of detected logical cores: %u\n", p_config->lcore_count);
    printf("Logical Core Configuration:\n");
    for (unsigned i = 0; i < RTE_MAX_LCORE; ++i) {
        struct lcore_config *lcore = &p_config->lcore_config[i];
        if (lcore->detected) {
            printf("Lcore ID: %u\n", i);
            printf("  Detected: %u\n", lcore->detected);
            printf("  Socket ID: %u\n", lcore->socket_id);
            printf("  Core ID: %u\n", lcore->core_id);
            printf("  Core Index: %d\n", lcore->core_index);
            // printf("  CPU Set: "); // Uncomment if you use cpuset
            // print_cpuset(lcore->cpuset); // Add a function to print cpuset if necessary
        }
    }
}