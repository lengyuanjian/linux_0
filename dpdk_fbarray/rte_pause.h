/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2017 Cavium, Inc
 */

#ifndef _RTE_PAUSE_H_
#define _RTE_PAUSE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <emmintrin.h>
static inline void rte_pause(void)
{
	_mm_pause();
}

#ifdef __cplusplus
}
#endif

#endif /* _RTE_PAUSE_H_ */
