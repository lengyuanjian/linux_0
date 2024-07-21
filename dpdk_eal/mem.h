#ifndef _SH_MEM_H_
#define _SH_MEM_H_
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include "eal_def.h"



#define MAX_HUGEPAGE_PATH PATH_MAX

typedef uint64_t phys_addr_t; /**< Physical address. */
#define RTE_BAD_PHYS_ADDR ((phys_addr_t)-1)
 
typedef uint64_t rte_iova_t;
#define RTE_BAD_IOVA ((rte_iova_t)-1) 

/**
 * Structure used to store informations about hugepages that we mapped
 * through the files in hugetlbfs.
 */
struct hugepage_file {
	void *orig_va;      /**< virtual addr of first mmap() */
	void *final_va;     /**< virtual addr of 2nd mmap() */
	uint64_t physaddr;  /**< physical addr */
	size_t size;        /**< the page size */
	int socket_id;      /**< NUMA socket ID */
	int file_id;        /**< the '%d' in HUGEFILE_FMT */
	char filepath[MAX_HUGEPAGE_PATH]; /**< path to backing file on filesystem */
};
#define RTE_MEMSEG_FLAG_DO_NOT_FREE (1 << 0)
struct rte_memseg {
	RTE_STD_C11
	union {
		phys_addr_t phys_addr;  /**< deprecated - Start physical address. */
		rte_iova_t iova;        /**< Start IO address. */
	};
	RTE_STD_C11
	union {
		void *addr;         /**< Start virtual address. */
		uint64_t addr_64;   /**< Makes sure addr is always 64 bits */
	};
	size_t len;               /**< Length of the segment. */
	uint64_t hugepage_sz;       /**< The pagesize of underlying memory */
	int32_t socket_id;          /**< NUMA socket ID. */
	uint32_t nchannel;          /**< Number of channels. */
	uint32_t nrank;             /**< Number of ranks. */
	uint32_t flags;             /**< Memseg-specific flags */
} __rte_packed;

#ifdef __cplusplus
extern "C" {
#endif


phys_addr_t rte_mem_virt2phy(const void *virtaddr);
enum rte_iova_mode rte_eal_iova_mode(void);
#ifdef __cplusplus
}
#endif

#endif