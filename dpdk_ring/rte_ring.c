/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2010-2015 Intel Corporation
 * Copyright (c) 2007,2008 Kip Macy kmacy@freebsd.org
 * All rights reserved.
 * Derived from FreeBSD's bufring.h
 * Used as BSD-3 Licensed with permission from Kip Macy.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "rte_ring.h"

 
/* true if x is a power of 2 */
#define POWEROF2(x) ((((x)-1) & (x)) == 0)

/* return the size of memory occupied by a ring */
ssize_t rte_ring_get_memsize(unsigned count)
{
	ssize_t sz;

	/* count must be a power of 2 */
	if ((!POWEROF2(count)) || (count > RTE_RING_SZ_MASK )) { 
		return 0;
	}

	sz = sizeof(struct rte_ring) + count * sizeof(void *);
	sz = RTE_ALIGN(sz, RTE_CACHE_LINE_SIZE);
	return sz;
}

void rte_ring_init(struct rte_ring *r, unsigned count, unsigned flags)
{
	memset(r, 0, sizeof(*r));
	r->flags = flags;
	r->prod.single = (flags & RING_F_SP_ENQ) ? __IS_SP : __IS_MP;
	r->cons.single = (flags & RING_F_SC_DEQ) ? __IS_SC : __IS_MC;

	r->size = count;
	r->mask = count - 1;
	r->capacity = r->mask;
	
	r->prod.head = r->cons.head = 0;
	r->prod.tail = r->cons.tail = 0; 
}

/* create the ring */
struct rte_ring *rte_ring_create(unsigned count, unsigned flags)
{
	struct rte_ring *r; 
	ssize_t ring_size; 

	ring_size = rte_ring_get_memsize(count);
	if (ring_size <= 0) { 
		return NULL;
	}

	r = (struct rte_ring *)malloc(ring_size);
	if (r == NULL) { 
		return NULL;
	}

	rte_ring_init(r, count, flags);

	return r;
}

/* free the ring */
void rte_ring_destory(struct rte_ring *r)
{
	 if(r)
	 {
		free(r);
	 }
}

/* dump the status of the ring on the console */
void rte_ring_dump(const struct rte_ring *r)
{
	// printf("ring <%s>@%p\n", r->name, r);
	printf("  flags=%x\n", r->flags);
	printf("  size=%"PRIu32"\n", r->size);
	printf("  capacity=%"PRIu32"\n", r->capacity);
	printf("  ct=%"PRIu32"\n", r->cons.tail);
	printf("  ch=%"PRIu32"\n", r->cons.head);
	printf("  pt=%"PRIu32"\n", r->prod.tail);
	printf("  ph=%"PRIu32"\n", r->prod.head);
	printf("  used=%u\n", rte_ring_count(r));
	printf("  avail=%u\n", rte_ring_free_count(r));
} 