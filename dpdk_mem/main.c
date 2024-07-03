#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include "../inc/sh_strings_fns.h"
// static const char sys_dir_path[] = "/sys/kernel/mm/hugepages";

// static int32_t
// get_num_hugepages(const char *subdir)
// {
// 	char path[PATH_MAX];
// 	long unsigned resv_pages, num_pages = 0;
// 	const char *nr_hp_file;
// 	const char *nr_rsvd_file = "resv_hugepages";

// 	/* first, check how many reserved pages kernel reports */
// 	snprintf(path, sizeof(path), "%s/%s/%s",
// 			sys_dir_path, subdir, nr_rsvd_file);

// 	if (eal_parse_sysfs_value(path, &resv_pages) < 0)
// 		return 0;

// 	/* if secondary process, just look at the number of hugepages,
// 	 * otherwise look at number of free hugepages */
// 	if (internal_config.process_type == RTE_PROC_SECONDARY)
// 		nr_hp_file = "nr_hugepages";
// 	else
// 		nr_hp_file = "free_hugepages";

// 	memset(path, 0, sizeof(path));

// 	snprintf(path, sizeof(path), "%s/%s/%s",
// 			sys_dir_path, subdir, nr_hp_file);

// 	if (eal_parse_sysfs_value(path, &num_pages) < 0)
// 		return 0;

// 	if (num_pages == 0)
// 		RTE_LOG(WARNING, EAL, "No free hugepages reported in %s\n",
// 				subdir);

// 	/* adjust num_pages in case of primary process */
// 	if (num_pages > 0 && internal_config.process_type == RTE_PROC_PRIMARY)
// 		num_pages -= resv_pages;

// 	return (int32_t)num_pages;
// }
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define RTE_MAX_NUMA_NODES  (1)
struct hugepage_info {
	uint64_t hugepage_sz;   /**< size of a huge page */
	char hugedir[PATH_MAX];    /**< dir where hugetlbfs is mounted */
	uint32_t num_pages[RTE_MAX_NUMA_NODES];
	/**< number of hugepages of that size on each socket */
	int lock_descriptor;    /**< file descriptor for hugepage dir */
};

static const char sys_dir_path[] = "/sys/kernel/mm/hugepages";


#include "mem.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
int main(int argc, char *argv[])
{
	printf("%s\n", argv[0]);
 
    const char dirent_start_text[] = "hugepages-";
	const size_t dirent_start_len = sizeof(dirent_start_text) - 1;
	unsigned int i, total_pages, num_sizes = 0;
	DIR *dir;
	struct dirent *dirent;

	dir = opendir(sys_dir_path);
	if (dir == NULL) {
		printf("Cannot open directory %s to read system hugepage info\n", sys_dir_path);
		return -1;
	}

	for (dirent = readdir(dir); dirent != NULL; dirent = readdir(dir)) 
	{
		
		if (strncmp(dirent->d_name, dirent_start_text, dirent_start_len) != 0)
			continue;
        if (num_sizes >= 3)
			break;
        num_sizes++;
		printf("size:[%lu]\n", rte_str_to_size(&dirent->d_name[dirent_start_len]));
        uint64_t page_size = rte_str_to_size(&dirent->d_name[dirent_start_len]);
		char name[64]={};
		if(0 != get_hugepage_dir(page_size, name ,64))
		{
			continue;
		}
		printf("getname[%s]\n",name);
		long unsigned num_pages = get_num_hugepages(dirent->d_name);
		printf("getname[%s][%lu]\n",name,num_pages);
    }
 
	size_t length = 6 * 1024 * 1024; // 2 MB

    // 使用 mmap 分配大页内存
    void *ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return 1;
    }

    // 使用分配的内存
    unsigned char *arr = (unsigned char *)ptr;
    arr[0] = 42;
	arr[1024 * 1024 * 2] = 42;
    printf("Value: %u\n", arr[0]);
	while(1)
	{;}
    // 释放内存
    if (munmap(ptr, length) == -1) {
        fprintf(stderr, "munmap failed: %s\n", strerror(errno));
        return 1;
    }


	return argc;
}
