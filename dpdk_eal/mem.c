#include "mem.h"
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
#include "mem_hugepage.h"

#define PFN_MASK_SIZE	8

static bool phys_addrs_available = true;

#define RANDOMIZE_VA_SPACE_FILE "/proc/sys/kernel/randomize_va_space"

enum rte_iova_mode rte_eal_iova_mode(void)
{
	return RTE_IOVA_PA;
}
static void
test_phys_addrs_available(void)
{
	uint64_t tmp = 0;
	phys_addr_t physaddr;

	// if (!rte_eal_has_hugepages()) {
	// 	RTE_LOG(ERR, EAL,
	// 		"Started without hugepages support, physical addresses not available\n");
	// 	phys_addrs_available = false;
	// 	return;
	// }

	physaddr = rte_mem_virt2phy(&tmp);
	if (physaddr == RTE_BAD_PHYS_ADDR) {
		if (rte_eal_iova_mode() == RTE_IOVA_PA)
		{
			RTE_LOG(ERR, EAL, "Cannot obtain physical addresses: %s. "
				"Only vfio will function.\n", strerror(errno));
		}
		phys_addrs_available = false;
	}
}

/*
 * Get physical address of any mapped virtual address in the current process.
 */
phys_addr_t
rte_mem_virt2phy(const void *virtaddr)
{
	int fd, retval;
	uint64_t page, physaddr;
	unsigned long virt_pfn;
	int page_size;
	off_t offset;

	/* Cannot parse /proc/self/pagemap, no need to log errors everywhere */
	if (!phys_addrs_available)
		return RTE_BAD_IOVA;

	/* standard page size */
	page_size = getpagesize();

	fd = open("/proc/self/pagemap", O_RDONLY);
	if (fd < 0) {
		RTE_LOG(ERR, EAL, "%s(): cannot open /proc/self/pagemap: %s\n",
			__func__, strerror(errno));
		return RTE_BAD_IOVA;
	}

	virt_pfn = (unsigned long)virtaddr / page_size;
	offset = sizeof(uint64_t) * virt_pfn;
	if (lseek(fd, offset, SEEK_SET) == (off_t) -1) {
		RTE_LOG(ERR, EAL, "%s(): seek error in /proc/self/pagemap: %s\n",
				__func__, strerror(errno));
		close(fd);
		return RTE_BAD_IOVA;
	}

	retval = read(fd, &page, PFN_MASK_SIZE);
	close(fd);
	if (retval < 0) {
		RTE_LOG(ERR, EAL, "%s(): cannot read /proc/self/pagemap: %s\n",
				__func__, strerror(errno));
		return RTE_BAD_IOVA;
	} else if (retval != PFN_MASK_SIZE) {
		RTE_LOG(ERR, EAL, "%s(): read %d bytes from /proc/self/pagemap "
				"but expected %d:\n",
				__func__, retval, PFN_MASK_SIZE);
		return RTE_BAD_IOVA;
	}

	/*
	 * the pfn (page frame number) are bits 0-54 (see
	 * pagemap.txt in linux Documentation)
	 */
	if ((page & 0x7fffffffffffffULL) == 0)
		return RTE_BAD_IOVA;

	physaddr = ((page & 0x7fffffffffffffULL) * page_size)
		+ ((unsigned long)virtaddr % page_size);

	return physaddr;
}

rte_iova_t
rte_mem_virt2iova(const void *virtaddr)
{
	if (rte_eal_iova_mode() == RTE_IOVA_VA)
		return (uintptr_t)virtaddr;
	return rte_mem_virt2phy(virtaddr);
}

/*
 * For each hugepage in hugepg_tbl, fill the physaddr value. We find
 * it by browsing the /proc/self/pagemap special file.
 */
static int
find_physaddrs(struct hugepage_file *hugepg_tbl, struct hugepage_info *hpi)
{
	unsigned int i;
	phys_addr_t addr;

	for (i = 0; i < hpi->num_pages[0]; i++) {
		addr = rte_mem_virt2phy(hugepg_tbl[i].orig_va);
		if (addr == RTE_BAD_PHYS_ADDR)
			return -1;
		hugepg_tbl[i].physaddr = addr;
	}
	return 0;
}

/*
 * For each hugepage in hugepg_tbl, fill the physaddr value sequentially.
 */
static int
set_physaddrs(struct hugepage_file *hugepg_tbl, struct hugepage_info *hpi)
{
	unsigned int i;
	static phys_addr_t addr;

	for (i = 0; i < hpi->num_pages[0]; i++) {
		hugepg_tbl[i].physaddr = addr;
		addr += hugepg_tbl[i].size;
	}
	return 0;
}


static int
calc_num_pages_per_socket(uint64_t * memory,
		struct hugepage_info *hp_info,
		struct hugepage_info *hp_used,
		unsigned num_hp_info)
{
	unsigned socket, j, i = 0;
	unsigned requested, available;
	int total_num_pages = 0;
	uint64_t remaining_mem, cur_mem;
	uint64_t total_mem = internal_config.memory;

	if (num_hp_info == 0)
		return -1;

	/* if specific memory amounts per socket weren't requested */
	if (internal_config.force_sockets == 0) {
		size_t total_size;
#ifdef RTE_ARCH_64
		int cpu_per_socket[RTE_MAX_NUMA_NODES];
		size_t default_size;
		unsigned lcore_id;

		/* Compute number of cores per socket */
		memset(cpu_per_socket, 0, sizeof(cpu_per_socket));
		RTE_LCORE_FOREACH(lcore_id) {
			cpu_per_socket[rte_lcore_to_socket_id(lcore_id)]++;
		}

		/*
		 * Automatically spread requested memory amongst detected sockets according
		 * to number of cores from cpu mask present on each socket
		 */
		total_size = internal_config.memory;
		for (socket = 0; socket < RTE_MAX_NUMA_NODES && total_size != 0; socket++) {

			/* Set memory amount per socket */
			default_size = (internal_config.memory * cpu_per_socket[socket])
					/ rte_lcore_count();

			/* Limit to maximum available memory on socket */
			default_size = RTE_MIN(default_size, get_socket_mem_size(socket));

			/* Update sizes */
			memory[socket] = default_size;
			total_size -= default_size;
		}

		/*
		 * If some memory is remaining, try to allocate it by getting all
		 * available memory from sockets, one after the other
		 */
		for (socket = 0; socket < RTE_MAX_NUMA_NODES && total_size != 0; socket++) {
			/* take whatever is available */
			default_size = RTE_MIN(get_socket_mem_size(socket) - memory[socket],
					       total_size);

			/* Update sizes */
			memory[socket] += default_size;
			total_size -= default_size;
		}
#else
		/* in 32-bit mode, allocate all of the memory only on master
		 * lcore socket
		 */
		total_size = internal_config.memory;
		for (socket = 0; socket < RTE_MAX_NUMA_NODES && total_size != 0;
				socket++) {
			struct rte_config *cfg = rte_eal_get_configuration();
			unsigned int master_lcore_socket;

			master_lcore_socket =
				rte_lcore_to_socket_id(cfg->master_lcore);

			if (master_lcore_socket != socket)
				continue;

			/* Update sizes */
			memory[socket] = total_size;
			break;
		}
#endif
	}

	for (socket = 0; socket < RTE_MAX_NUMA_NODES && total_mem != 0; socket++) {
		/* skips if the memory on specific socket wasn't requested */
		for (i = 0; i < num_hp_info && memory[socket] != 0; i++){
			strlcpy(hp_used[i].hugedir, hp_info[i].hugedir,
				sizeof(hp_used[i].hugedir));
			hp_used[i].num_pages[socket] = RTE_MIN(
					memory[socket] / hp_info[i].hugepage_sz,
					hp_info[i].num_pages[socket]);

			cur_mem = hp_used[i].num_pages[socket] *
					hp_used[i].hugepage_sz;

			memory[socket] -= cur_mem;
			total_mem -= cur_mem;

			total_num_pages += hp_used[i].num_pages[socket];

			/* check if we have met all memory requests */
			if (memory[socket] == 0)
				break;

			/* check if we have any more pages left at this size, if so
			 * move on to next size */
			if (hp_used[i].num_pages[socket] == hp_info[i].num_pages[socket])
				continue;
			/* At this point we know that there are more pages available that are
			 * bigger than the memory we want, so lets see if we can get enough
			 * from other page sizes.
			 */
			remaining_mem = 0;
			for (j = i+1; j < num_hp_info; j++)
				remaining_mem += hp_info[j].hugepage_sz *
				hp_info[j].num_pages[socket];

			/* is there enough other memory, if not allocate another page and quit */
			if (remaining_mem < memory[socket]){
				cur_mem = RTE_MIN(memory[socket],
						hp_info[i].hugepage_sz);
				memory[socket] -= cur_mem;
				total_mem -= cur_mem;
				hp_used[i].num_pages[socket]++;
				total_num_pages++;
				break; /* we are done with this socket*/
			}
		}
		/* if we didn't satisfy all memory requirements per socket */
		if (memory[socket] > 0 &&
				internal_config.socket_mem[socket] != 0) {
			/* to prevent icc errors */
			requested = (unsigned) (internal_config.socket_mem[socket] /
					0x100000);
			available = requested -
					((unsigned) (memory[socket] / 0x100000));
			RTE_LOG(ERR, EAL, "Not enough memory available on socket %u! "
					"Requested: %uMB, available: %uMB\n", socket,
					requested, available);
			return -1;
		}
	}

	/* if we didn't satisfy total memory requirements */
	if (total_mem > 0) {
		requested = (unsigned) (internal_config.memory / 0x100000);
		available = requested - (unsigned) (total_mem / 0x100000);
		RTE_LOG(ERR, EAL, "Not enough memory available! Requested: %uMB,"
				" available: %uMB\n", requested, available);
		return -1;
	}
	return total_num_pages;
}


static int
eal_hugepage_init(void)
{
	struct hugepage_info used_hp[MAX_HUGEPAGE_SIZES];
	uint64_t memory[RTE_MAX_NUMA_NODES];
	int hp_sz_idx, socket_id;

	test_phys_addrs_available();

	memset(used_hp, 0, sizeof(used_hp));

	for (hp_sz_idx = 0; hp_sz_idx < (int) mem_hugepage_conf.num_hugepage_sizes; hp_sz_idx++) 
	{

		/* also initialize used_hp hugepage sizes in used_hp */
		struct hugepage_info *hpi;
		hpi = &mem_hugepage_conf.hugepage_info[hp_sz_idx];
		used_hp[hp_sz_idx].hugepage_sz = hpi->hugepage_sz;


	}

	// /* make a copy of socket_mem, needed for balanced allocation. */
	// for (hp_sz_idx = 0; hp_sz_idx < RTE_MAX_NUMA_NODES; hp_sz_idx++)
	// 	memory[hp_sz_idx] = internal_config.socket_mem[hp_sz_idx];

	/* calculate final number of pages */
	if (calc_num_pages_per_socket(memory, mem_hugepage_conf.hugepage_info, used_hp, mem_hugepage_conf.num_hugepage_sizes) < 0)
		return -1;

	for (hp_sz_idx = 0; hp_sz_idx < (int)mem_hugepage_conf.num_hugepage_sizes; hp_sz_idx++) 
	{
		for (socket_id = 0; socket_id < RTE_MAX_NUMA_NODES;	socket_id++) 
		{
			struct rte_memseg **pages;
			struct hugepage_info *hpi = &used_hp[hp_sz_idx];
			unsigned int num_pages = hpi->num_pages[socket_id];
			int num_pages_alloc, i;

			if (num_pages == 0)
				continue;

			pages = malloc(sizeof(*pages) * num_pages);

			RTE_LOG(DEBUG, EAL, "Allocating %u pages of size %" PRIu64 "M on socket %i\n",
				num_pages, hpi->hugepage_sz >> 20, socket_id);

			num_pages_alloc = eal_memalloc_alloc_seg_bulk(pages,
					num_pages, hpi->hugepage_sz,
					socket_id, true);
			if (num_pages_alloc < 0) 
			{
				free(pages);
				return -1;
			}

			/* mark preallocated pages as unfreeable */
			for (i = 0; i < num_pages_alloc; i++) 
			{
				struct rte_memseg *ms = pages[i];
				ms->flags |= RTE_MEMSEG_FLAG_DO_NOT_FREE;
			}
			free(pages);
		}
	}
	return 0;
}