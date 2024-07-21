#ifndef _EAL_DEF_H_
#define _EAL_DEF_H_

#define RTE_MAX_LCORE            (128)
#define RTE_MAX_NUMA_NODES       (8)
#define	RTE_DIM(a)	(sizeof (a) / sizeof ((a)[0]))

#define RTE_ARCH_64

/**
 * Force alignment
 */
#define __rte_aligned(a) __attribute__((__aligned__(a)))

/**
 * Force a structure to be packed
 */
#define __rte_packed __attribute__((__packed__))

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

#ifndef typeof
#define typeof __typeof__
#endif

#ifndef asm
#define asm __asm__
#endif

/** C extension macro for environments lacking C11 features. */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#define RTE_STD_C11 __extension__
#else
#define RTE_STD_C11
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define bool _Bool
#define true 1
#define false 0

enum rte_iova_mode {
	RTE_IOVA_DC = 0,	/* Don't care mode */
	RTE_IOVA_PA = (1 << 0), /* DMA using physical address */
	RTE_IOVA_VA = (1 << 1)  /* DMA using virtual address */
};

extern struct eal_config _eal_config;

extern int eal_cpu_init(void);

extern unsigned int rte_socket_count();

extern int rte_socket_id_by_idx(unsigned int idx);

extern void printf_eal_config(struct eal_config * p_config);



#ifdef __cplusplus
}
#endif

#endif