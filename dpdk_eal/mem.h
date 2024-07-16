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

#define MAX_HUGEPAGE_SIZES 3  /**< support up to 3 page sizes */

struct hugepage_info {
	uint64_t hugepage_sz;   /**< size of a huge page */
	char hugedir[PATH_MAX];    /**< dir where hugetlbfs is mounted */
	uint32_t num_pages[RTE_MAX_NUMA_NODES];
	/**< number of hugepages of that size on each socket */
	int lock_descriptor;    /**< file descriptor for hugepage dir */
};

struct hugepage_conf
{
	const char *hugefile_prefix;      /**< the base filename of hugetlbfs files */
	const char *hugepage_dir;         /**< specific hugetlbfs directory to use */
	unsigned num_hugepage_sizes;      /**< how many sizes on this system */
	struct hugepage_info hugepage_info[MAX_HUGEPAGE_SIZES];
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct hugepage_conf mem_conf;

#ifdef __cplusplus
}
#endif

#endif