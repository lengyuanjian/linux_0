#include "mem_hugepage.h"
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fnmatch.h>
#include <inttypes.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include "../inc/sh_unistd.h"
#include "eal_log.h"

static const char sys_dir_path[] = "/sys/kernel/mm/hugepages";
static const char sys_pages_numa_dir_path[] = "/sys/devices/system/node";

struct hugepage_conf mem_hugepage_conf;

int printf_mem_conf(struct hugepage_conf *p_mem_conf) 
{
    if (p_mem_conf == NULL) {
        return -1; 
    }

    printf("Hugefile Prefix: %s\n", p_mem_conf->hugefile_prefix);
    printf("Hugepage Directory: %s\n", p_mem_conf->hugepage_dir);
    printf("Number of Hugepage Sizes: %u\n", p_mem_conf->num_hugepage_sizes);

    for (unsigned i = 0; i < p_mem_conf->num_hugepage_sizes; ++i) {
        struct hugepage_info *info = &p_mem_conf->hugepage_info[i];
        printf("Hugepage Size %u: %lu\n", i, info->hugepage_sz);
        printf("Hugepage Directory: %s\n", info->hugedir);
        printf("Number of Pages per NUMA Node:\n");
        for (unsigned j = 0; j < info->numa_node_count; ++j) {
            printf("  Node %u: %u pages\n", j, info->num_pages[j]);
        }
        printf("Lock Descriptor: %d\n", info->lock_descriptor);
    }

    return 0;
}

static uint64_t
get_default_hp_size(void)
{
	const char proc_meminfo[] = "/proc/meminfo";
	const char str_hugepagesz[] = "Hugepagesize:";
	unsigned hugepagesz_len = sizeof(str_hugepagesz) - 1;
	char buffer[256];
	unsigned long long size = 0;

	FILE *fd = fopen(proc_meminfo, "r");
	if (fd == NULL)
		rte_panic("Cannot open %s\n", proc_meminfo);
	while(fgets(buffer, sizeof(buffer), fd)){
		if (strncmp(buffer, str_hugepagesz, hugepagesz_len) == 0){
			size = rte_str_to_size(&buffer[hugepagesz_len]);
			break;
		}
	}
	fclose(fd);
	if (size == 0)
		rte_panic("Cannot get default hugepage size from %s\n", proc_meminfo);
	return size;
}

static int
get_hugepage_dir(uint64_t hugepage_sz, char *hugedir, int len)
{
	enum proc_mount_fieldnames {
		DEVICE = 0,
		MOUNTPT,
		FSTYPE,
		OPTIONS,
		_FIELDNAME_MAX
	};
	static uint64_t default_size = 0;
	const char proc_mounts[] = "/proc/mounts";
	const char hugetlbfs_str[] = "hugetlbfs";
	const size_t htlbfs_str_len = sizeof(hugetlbfs_str) - 1;
	const char pagesize_opt[] = "pagesize=";
	const size_t pagesize_opt_len = sizeof(pagesize_opt) - 1;
	const char split_tok = ' ';
	char *splitstr[_FIELDNAME_MAX];
	char buf[BUFSIZ];
	int retval = -1;

	FILE *fd = fopen(proc_mounts, "r");
	if (fd == NULL)
		rte_panic("Cannot open %s\n", proc_mounts);

	if (default_size == 0)
		default_size = get_default_hp_size();

	while (fgets(buf, sizeof(buf), fd)){
		if (rte_strsplit(buf, sizeof(buf), splitstr, _FIELDNAME_MAX,
				split_tok) != _FIELDNAME_MAX) {
			RTE_LOG(ERR, EAL, "Error parsing %s\n", proc_mounts);
			break; /* return NULL */
		}

		// /* we have a specified --huge-dir option, only examine that dir */
		// if (internal_config.hugepage_dir != NULL &&
		// 		strcmp(splitstr[MOUNTPT], internal_config.hugepage_dir) != 0)
		// 	continue;

		if (strncmp(splitstr[FSTYPE], hugetlbfs_str, htlbfs_str_len) == 0){
			const char *pagesz_str = strstr(splitstr[OPTIONS], pagesize_opt);

			/* if no explicit page size, the default page size is compared */
			if (pagesz_str == NULL){
				if (hugepage_sz == default_size){
					strlcpy(hugedir, splitstr[MOUNTPT], len);
					retval = 0;
					break;
				}
			}
			/* there is an explicit page size, so check it */
			else {
				uint64_t pagesz = rte_str_to_size(&pagesz_str[pagesize_opt_len]);
				if (pagesz == hugepage_sz) {
					strlcpy(hugedir, splitstr[MOUNTPT], len);
					retval = 0;
					break;
				}
			}
		} /* end if strncmp hugetlbfs */
	} /* end while fgets */

	fclose(fd);
	return retval;
}

static uint32_t
get_num_hugepages(const char *subdir)
{
	char path[PATH_MAX];
	long unsigned resv_pages, num_pages = 0;
	const char *nr_hp_file = "free_hugepages";
	const char *nr_rsvd_file = "resv_hugepages";

	/* first, check how many reserved pages kernel reports */
	snprintf(path, sizeof(path), "%s/%s/%s",
			sys_dir_path, subdir, nr_rsvd_file);
	if (eal_parse_sysfs_value(path, &resv_pages) < 0)
		return 0;

	snprintf(path, sizeof(path), "%s/%s/%s",
			sys_dir_path, subdir, nr_hp_file);
	if (eal_parse_sysfs_value(path, &num_pages) < 0)
		return 0;

	if (num_pages == 0)
		RTE_LOG(WARNING, EAL, "No free hugepages reported in %s\n",
				subdir);

	/* adjust num_pages */
	if (num_pages >= resv_pages)
		num_pages -= resv_pages;
	else if (resv_pages)
		num_pages = 0;

	/* we want to return a uint32_t and more than this looks suspicious
	 * anyway ... */
	if (num_pages > UINT32_MAX)
		num_pages = UINT32_MAX;

	return num_pages;
}


static int
clear_hugedir(const char * hugedir)
{
	DIR *dir;
	struct dirent *dirent;
	int dir_fd, fd, lck_result;
	const char filter[] = "*map_*"; /* matches hugepage files */

	/* open directory */
	dir = opendir(hugedir);
	if (!dir) {
		RTE_LOG(ERR, EAL, "Unable to open hugepage directory %s\n",
				hugedir);
		goto error;
	}
	dir_fd = dirfd(dir);

	dirent = readdir(dir);
	if (!dirent) {
		RTE_LOG(ERR, EAL, "Unable to read hugepage directory %s\n",
				hugedir);
		goto error;
	}

	while(dirent != NULL){
		/* skip files that don't match the hugepage pattern */
		if (fnmatch(filter, dirent->d_name, 0) > 0) {
			dirent = readdir(dir);
			continue;
		}

		/* try and lock the file */
		fd = openat(dir_fd, dirent->d_name, O_RDONLY);

		/* skip to next file */
		if (fd == -1) {
			dirent = readdir(dir);
			continue;
		}

		/* non-blocking lock */
		lck_result = flock(fd, LOCK_EX | LOCK_NB);

		/* if lock succeeds, remove the file */
		if (lck_result != -1)
			unlinkat(dir_fd, dirent->d_name, 0);
		close (fd);
		dirent = readdir(dir);
	}

	closedir(dir);
	return 0;

error:
	if (dir)
		closedir(dir);

	RTE_LOG(ERR, EAL, "Error while clearing hugepage dir: %s\n",
		strerror(errno));

	return -1;
}

static int
compare_hpi(const void *a, const void *b)
{
	const struct hugepage_info *hpi_a = a;
	const struct hugepage_info *hpi_b = b;

	return hpi_b->hugepage_sz - hpi_a->hugepage_sz;
}

static uint32_t
get_num_hugepages_on_node(const char *subdir, unsigned int socket)
{
	char path[PATH_MAX * 3], socketpath[PATH_MAX];
	DIR *socketdir;
	unsigned long num_pages = 0;
	const char *nr_hp_file = "free_hugepages";

	snprintf(socketpath, sizeof(socketpath), "%s/node%u/hugepages",
		sys_pages_numa_dir_path, socket);

	socketdir = opendir(socketpath);
	if (socketdir) {
		/* Keep calm and carry on */
		closedir(socketdir);
	} else {
		/* Can't find socket dir, so ignore it */
		return 0;
	}

	snprintf(path, sizeof(path), "%s/%s/%s",
			socketpath, subdir, nr_hp_file);
	if (eal_parse_sysfs_value(path, &num_pages) < 0)
		return 0;

	if (num_pages == 0)
		RTE_LOG(WARNING, EAL, "No free hugepages reported in %s\n",
				subdir);

	/*
	 * we want to return a uint32_t and more than this looks suspicious
	 * anyway ...
	 */
	if (num_pages > UINT32_MAX)
		num_pages = UINT32_MAX;

	return num_pages;
}

int hugepage_info_init(void)
{	const char dirent_start_text[] = "hugepages-";
	const size_t dirent_start_len = sizeof(dirent_start_text) - 1;
	unsigned int i, total_pages, num_sizes = 0;
	DIR *dir;
	struct dirent *dirent;

	dir = opendir(sys_dir_path);
	if (dir == NULL) {
		RTE_LOG(ERR, EAL,
			"Cannot open directory %s to read system hugepage info\n",
			sys_dir_path);
		return -1;
	}

	for (dirent = readdir(dir); dirent != NULL; dirent = readdir(dir)) 
    {
		struct hugepage_info *hpi;

		if (strncmp(dirent->d_name, dirent_start_text, dirent_start_len) != 0)
			continue;

		if (num_sizes >= MAX_HUGEPAGE_SIZES)
			break;

		hpi = &mem_hugepage_conf.hugepage_info[num_sizes];
		hpi->hugepage_sz = rte_str_to_size(&dirent->d_name[dirent_start_len]);

		/* first, check if we have a mountpoint */
		if (get_hugepage_dir(hpi->hugepage_sz, hpi->hugedir, sizeof(hpi->hugedir)) < 0) 
        {
			uint32_t num_pages;

			num_pages = get_num_hugepages(dirent->d_name);
			if (num_pages > 0)
            {
				RTE_LOG(NOTICE, EAL,
					"%" PRIu32 " hugepages of size "
					"%" PRIu64 " reserved, but no mounted "
					"hugetlbfs found for that size\n",
					num_pages, hpi->hugepage_sz);
            }
			continue;
		}

		/* try to obtain a writelock */
		hpi->lock_descriptor = open(hpi->hugedir, O_RDONLY);

		/* if blocking lock failed */
		if (flock(hpi->lock_descriptor, LOCK_EX) == -1) 
        {
			RTE_LOG(CRIT, EAL,
				"Failed to lock hugepage directory!\n");
			break;
		}
		/* clear out the hugepages dir from unused pages */
		if (clear_hugedir(hpi->hugedir) == -1)
			break;

		/*
		 * first, try to put all hugepages into relevant sockets, but
		 * if first attempts fails, fall back to collecting all pages
		 * in one socket and sorting them later
		 */
		total_pages = 0;
		/* we also don't want to do this for legacy init */
		//if (!internal_config.legacy_mem)
        hpi->numa_node_count = rte_socket_count();
		for (i = 0; i < hpi->numa_node_count; i++) 
        {
            int socket = rte_socket_id_by_idx(i);
            if(socket == -1)
            {
                RTE_LOG(ERR, EAL, "socket select failed! id:%d\n", i);
            }
            unsigned int num_pages = get_num_hugepages_on_node(dirent->d_name, socket);
            hpi->num_pages[socket] = num_pages;
            total_pages += num_pages;
		}
		/*
		 * we failed to sort memory from the get go, so fall
		 * back to old way
		 */
		if (total_pages == 0)
			hpi->num_pages[0] = get_num_hugepages(dirent->d_name);

		num_sizes++;
	}
	closedir(dir);

	/* something went wrong, and we broke from the for loop above */
	if (dirent != NULL)
		return -1;

	mem_hugepage_conf.num_hugepage_sizes = num_sizes;

	/* sort the page directory entries by size, largest to smallest */
	qsort(&mem_hugepage_conf.hugepage_info[0], num_sizes,
	      sizeof(mem_hugepage_conf.hugepage_info[0]), compare_hpi);

	/* now we have all info, check we have at least one valid size */
	for (i = 0; i < num_sizes; i++) {
		/* pages may no longer all be on socket 0, so check all */
		unsigned int j, num_pages = 0;
		struct hugepage_info *hpi = &mem_hugepage_conf.hugepage_info[i];

		for (j = 0; j < RTE_MAX_NUMA_NODES; j++)
			num_pages += hpi->num_pages[j];
		if (strnlen(hpi->hugedir, sizeof(hpi->hugedir)) != 0 &&
				num_pages > 0)
			return 0;
	}

	/* no valid hugepage mounts available, return error */
	return -1;
}