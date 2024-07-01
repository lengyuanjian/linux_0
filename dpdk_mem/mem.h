#ifndef _SH_MEM_H_
#define _SH_MEM_H_
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "../inc/sh_strings_fns.h"
#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t
rte_str_to_size(const char *str)
{
	char *endptr;
	unsigned long long size;

	while (isspace((int)*str))
		str++;
	if (*str == '-')
		return 0;

	errno = 0;
	size = strtoull(str, &endptr, 0);
	if (errno)
		return 0;

	if (*endptr == ' ')
		endptr++; /* allow 1 space gap */

	switch (*endptr){
	case 'G': case 'g': size *= 1024; /* fall-through */
	case 'M': case 'm': size *= 1024; /* fall-through */
	case 'K': case 'k': size *= 1024; /* fall-through */
	default:
		break;
	}
	return size;
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
	{
        	printf("Cannot open %s\n", proc_meminfo);
            return 0;
    }
	while(fgets(buffer, sizeof(buffer), fd))
    {
		if (strncmp(buffer, str_hugepagesz, hugepagesz_len) == 0)
        {
			size = rte_str_to_size(&buffer[hugepagesz_len]);
			break;
		}
	}
	fclose(fd);
	if (size == 0)
	{
        printf("Cannot get default hugepage size from %s\n", proc_meminfo);
        return 0;
    }
	return size;
}

static inline int get_hugepage_dir(uint64_t hugepage_sz, char *hugedir, int len)
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
		printf("Cannot open %s\n", proc_mounts);

    default_size = get_default_hp_size();
	if (default_size == 0)
		return retval;
    printf("def mem size[%lu]\n",default_size);
	while (fgets(buf, sizeof(buf), fd))
    {
        // hugetlbfs /mnt/huge hugetlbfs rw,relatime,pagesize=2M 0 0
		if (sh_strsplit(buf, sizeof(buf), splitstr, _FIELDNAME_MAX,split_tok) != _FIELDNAME_MAX) 
        {
			printf("Error parsing %s\n", proc_mounts);
			break; /* return NULL */
		}

		/* we have a specified --huge-dir option, only examine that dir */
		// if (NULL &&  strcmp(splitstr[MOUNTPT], internal_config.hugepage_dir) != 0)
		// 	continue;

		if (strncmp(splitstr[FSTYPE], hugetlbfs_str, htlbfs_str_len) == 0)
        {
			const char *pagesz_str = strstr(splitstr[OPTIONS], pagesize_opt);

			/* if no explicit page size, the default page size is compared */
			if (pagesz_str == NULL)
            {
				if (hugepage_sz == default_size)
                {
					snprintf(hugedir, len,"%s" ,splitstr[MOUNTPT]);
					retval = 0;
					break;
				}
			}
			/* there is an explicit page size, so check it */
			else 
            {
				uint64_t pagesz = rte_str_to_size(&pagesz_str[pagesize_opt_len]);
				if (pagesz == hugepage_sz) 
                {
					snprintf(hugedir,  len,"%s" , splitstr[MOUNTPT] );
					retval = 0;
					break;
				}
			}
		} /* end if strncmp hugetlbfs */
	} /* end while fgets */

	fclose(fd);
	return retval;
}

int
eal_parse_sysfs_value(const char *filename, unsigned long *val)
{
	FILE *f;
	char buf[BUFSIZ];
	char *end = NULL;

	if ((f = fopen(filename, "r")) == NULL) {
		printf( "%s(): cannot open sysfs value %s\n",
			__func__, filename);
		return -1;
	}

	if (fgets(buf, sizeof(buf), f) == NULL) {
		printf( "%s(): cannot read sysfs value %s\n",
			__func__, filename);
		fclose(f);
		return -1;
	}
	*val = strtoul(buf, &end, 0);
	if ((buf[0] == '\0') || (end == NULL) || (*end != '\n')) {
		printf( "%s(): cannot parse sysfs value %s\n",
				__func__, filename);
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

static uint32_t
get_num_hugepages(const char *subdir)
{
    const char sys_dir_path[] = "/sys/kernel/mm/hugepages";
	char path[PATH_MAX];
	long unsigned resv_pages, num_pages = 0;
	const char *nr_hp_file = "free_hugepages";
	const char *nr_rsvd_file = "resv_hugepages";

	/* first, check how many reserved pages kernel reports */
	snprintf(path, sizeof(path), "%s/%s/%s",
			sys_dir_path, subdir, nr_rsvd_file);
	if (eal_parse_sysfs_value(path, &resv_pages) < 0)
		return 0;

	snprintf(path, sizeof(path), "%s/%s/%s",  sys_dir_path, subdir, nr_hp_file);
	if (eal_parse_sysfs_value(path, &num_pages) < 0)
		return 0;

	if (num_pages == 0)
		printf( "No free hugepages reported in %s\n",
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

#ifdef __cplusplus
}
#endif

#endif