#ifndef _EAL_DEF_H_
#define _EAL_DEF_H_

#define RTE_MAX_LCORE            (128)
#define RTE_MAX_NUMA_NODES       (8)
#define	RTE_DIM(a)	(sizeof (a) / sizeof ((a)[0]))
struct lcore_config
{
    unsigned detected;         /**< true if lcore was detected */
    unsigned socket_id;        /**< physical socket id for this lcore */
	unsigned core_id;          /**< core number on socket for this lcore */
	int core_index;            /**< relative index, starting from 0 */
    // rte_cpuset_t cpuset;       /**< cpu set which the lcore affinity to */
};

struct eal_config
{
    unsigned numa_node_count;    /**< Number of detected NUMA nodes. */
	unsigned numa_nodes[RTE_MAX_NUMA_NODES]; /**< List of detected NUMA nodes. */
    unsigned lcore_count;
    struct lcore_config lcore_config[RTE_MAX_LCORE];
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct eal_config _eal_config;

extern int eal_cpu_init(void);

extern unsigned int rte_socket_count();

extern int rte_socket_id_by_idx(unsigned int idx);

extern void printf_eal_config(struct eal_config * p_config);



#ifdef __cplusplus
}
#endif

#endif