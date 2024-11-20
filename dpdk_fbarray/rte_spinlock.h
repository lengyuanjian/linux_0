/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#ifndef _RTE_SPINLOCK_H_
#define _RTE_SPINLOCK_H_

/**
 * @file
 *
 * RTE Spinlocks
 *
 * This file defines an API for read-write locks, which are implemented
 * in an architecture-specific way. This kind of lock simply waits in
 * a loop repeatedly checking until the lock becomes available.
 *
 * All locks must be initialised before use, and only initialised once.
 *
 */

#include <rte_lcore.h>
#ifdef RTE_FORCE_INTRINSICS
#include "rte_common.h"
#endif
#include "rte_pause.h"

/**
 * The rte_spinlock_t type.
 */
typedef struct {
	volatile int locked; /**< lock status 0 = unlocked, 1 = locked */
} rte_spinlock_t;

/**
 * A static spinlock initializer.
 */
#define RTE_SPINLOCK_INITIALIZER { 0 }

/**
 * Initialize the spinlock to an unlocked state.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
rte_spinlock_init(rte_spinlock_t *sl)
{
	sl->locked = 0;
}

/**
 * Take the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
rte_spinlock_lock(rte_spinlock_t *sl);

#ifdef RTE_FORCE_INTRINSICS
static inline void
rte_spinlock_lock(rte_spinlock_t *sl)
{
	while (__sync_lock_test_and_set(&sl->locked, 1))
		while(sl->locked)
			rte_pause();
}
#endif

/**
 * Release the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
rte_spinlock_unlock (rte_spinlock_t *sl);

#ifdef RTE_FORCE_INTRINSICS
static inline void
rte_spinlock_unlock (rte_spinlock_t *sl)
{
	__sync_lock_release(&sl->locked);
}
#endif

/**
 * Try to take the lock.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int
rte_spinlock_trylock (rte_spinlock_t *sl);

#ifdef RTE_FORCE_INTRINSICS
static inline int
rte_spinlock_trylock (rte_spinlock_t *sl)
{
	return __sync_lock_test_and_set(&sl->locked,1) == 0;
}
#endif

/**
 * Test if the lock is taken.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is currently taken; 0 otherwise.
 */
static inline int rte_spinlock_is_locked (rte_spinlock_t *sl)
{
	return sl->locked;
}

/**
 * Test if hardware transactional memory (lock elision) is supported
 *
 * @return
 *   1 if the hardware transactional memory is supported; 0 otherwise.
 */
static inline int rte_tm_supported(void);

/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available take the spinlock.
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
rte_spinlock_lock_tm(rte_spinlock_t *sl);

/**
 * Commit hardware memory transaction or release the spinlock if
 * the spinlock is used as a fall-back
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
rte_spinlock_unlock_tm(rte_spinlock_t *sl);

/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available try to take the lock.
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the hardware memory transaction is successfully started
 *   or lock is successfully taken; 0 otherwise.
 */
static inline int
rte_spinlock_trylock_tm(rte_spinlock_t *sl);

/**
 * The rte_spinlock_recursive_t type.
 */
typedef struct {
	rte_spinlock_t sl; /**< the actual spinlock */
	volatile int user; /**< core id using lock, -1 for unused */
	volatile int count; /**< count of time this lock has been called */
} rte_spinlock_recursive_t;

/**
 * A static recursive spinlock initializer.
 */
#define RTE_SPINLOCK_RECURSIVE_INITIALIZER {RTE_SPINLOCK_INITIALIZER, -1, 0}

/**
 * Initialize the recursive spinlock to an unlocked state.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void rte_spinlock_recursive_init(rte_spinlock_recursive_t *slr)
{
	rte_spinlock_init(&slr->sl);
	slr->user = -1;
	slr->count = 0;
}

/**
 * Take the recursive spinlock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void rte_spinlock_recursive_lock(rte_spinlock_recursive_t *slr)
{
	int id = rte_gettid();

	if (slr->user != id) {
		rte_spinlock_lock(&slr->sl);
		slr->user = id;
	}
	slr->count++;
}
/**
 * Release the recursive spinlock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void rte_spinlock_recursive_unlock(rte_spinlock_recursive_t *slr)
{
	if (--(slr->count) == 0) {
		slr->user = -1;
		rte_spinlock_unlock(&slr->sl);
	}

}

/**
 * Try to take the recursive lock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int rte_spinlock_recursive_trylock(rte_spinlock_recursive_t *slr)
{
	int id = rte_gettid();

	if (slr->user != id) {
		if (rte_spinlock_trylock(&slr->sl) == 0)
			return 0;
		slr->user = id;
	}
	slr->count++;
	return 1;
}


/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available take the recursive spinlocks
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void rte_spinlock_recursive_lock_tm(
	rte_spinlock_recursive_t *slr);

/**
 * Commit hardware memory transaction or release the recursive spinlock
 * if the recursive spinlock is used as a fall-back
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void rte_spinlock_recursive_unlock_tm(
	rte_spinlock_recursive_t *slr);

/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available try to take the recursive lock
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 * @return
 *   1 if the hardware memory transaction is successfully started
 *   or lock is successfully taken; 0 otherwise.
 */
static inline int rte_spinlock_recursive_trylock_tm(
	rte_spinlock_recursive_t *slr);

#ifdef __cplusplus
extern "C" {
#endif

#define RTE_RTM_MAX_RETRIES (10)
#define RTE_XABORT_LOCK_BUSY (0xff)

#ifndef RTE_FORCE_INTRINSICS
static inline void
rte_spinlock_lock(rte_spinlock_t *sl)
{
	int lock_val = 1;
	asm volatile (
			"1:\n"
			"xchg %[locked], %[lv]\n"
			"test %[lv], %[lv]\n"
			"jz 3f\n"
			"2:\n"
			"pause\n"
			"cmpl $0, %[locked]\n"
			"jnz 2b\n"
			"jmp 1b\n"
			"3:\n"
			: [locked] "=m" (sl->locked), [lv] "=q" (lock_val)
			: "[lv]" (lock_val)
			: "memory");
}

static inline void
rte_spinlock_unlock (rte_spinlock_t *sl)
{
	int unlock_val = 0;
	asm volatile (
			"xchg %[locked], %[ulv]\n"
			: [locked] "=m" (sl->locked), [ulv] "=q" (unlock_val)
			: "[ulv]" (unlock_val)
			: "memory");
}

static inline int
rte_spinlock_trylock (rte_spinlock_t *sl)
{
	int lockval = 1;

	asm volatile (
			"xchg %[locked], %[lockval]"
			: [locked] "=m" (sl->locked), [lockval] "=q" (lockval)
			: "[lockval]" (lockval)
			: "memory");

	return lockval == 0;
}
#endif

extern uint8_t rte_rtm_supported;

static inline int rte_tm_supported(void)
{
	return rte_rtm_supported;
}

static inline int
rte_try_tm(volatile int *lock)
{
	int retries;

	if (!rte_rtm_supported)
		return 0;

	retries = RTE_RTM_MAX_RETRIES;

	while (likely(retries--)) {

		unsigned int status = rte_xbegin();

		if (likely(RTE_XBEGIN_STARTED == status)) {
			if (unlikely(*lock))
				rte_xabort(RTE_XABORT_LOCK_BUSY);
			else
				return 1;
		}
		while (*lock)
			rte_pause();

		if ((status & RTE_XABORT_EXPLICIT) &&
			(RTE_XABORT_CODE(status) == RTE_XABORT_LOCK_BUSY))
			continue;

		if ((status & RTE_XABORT_RETRY) == 0) /* do not retry */
			break;
	}
	return 0;
}

static inline void
rte_spinlock_lock_tm(rte_spinlock_t *sl)
{
	if (likely(rte_try_tm(&sl->locked)))
		return;

	rte_spinlock_lock(sl); /* fall-back */
}

static inline int
rte_spinlock_trylock_tm(rte_spinlock_t *sl)
{
	if (likely(rte_try_tm(&sl->locked)))
		return 1;

	return rte_spinlock_trylock(sl);
}

static inline void
rte_spinlock_unlock_tm(rte_spinlock_t *sl)
{
	if (unlikely(sl->locked))
		rte_spinlock_unlock(sl);
	else
		rte_xend();
}

static inline void
rte_spinlock_recursive_lock_tm(rte_spinlock_recursive_t *slr)
{
	if (likely(rte_try_tm(&slr->sl.locked)))
		return;

	rte_spinlock_recursive_lock(slr); /* fall-back */
}

static inline void
rte_spinlock_recursive_unlock_tm(rte_spinlock_recursive_t *slr)
{
	if (unlikely(slr->sl.locked))
		rte_spinlock_recursive_unlock(slr);
	else
		rte_xend();
}

static inline int
rte_spinlock_recursive_trylock_tm(rte_spinlock_recursive_t *slr)
{
	if (likely(rte_try_tm(&slr->sl.locked)))
		return 1;

	return rte_spinlock_recursive_trylock(slr);
}


#ifdef __cplusplus
}
#endif

#endif /* _RTE_SPINLOCK_H_ */
