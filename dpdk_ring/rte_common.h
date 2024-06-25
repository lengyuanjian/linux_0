#ifndef _RTE_COMMON_H_
#define _RTE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ENOBUFS
#define ENOBUFS 1
#endif

#ifndef ENOENT
#define ENOENT 1
#endif

#define RTE_CACHE_LINE_SIZE (64)                     

#define RTE_CACHE_LINE_MASK (RTE_CACHE_LINE_SIZE-1) /**< Cache line mask. */

#define __rte_aligned(a) __attribute__((__aligned__(a)))
#define __rte_cache_aligned __rte_aligned(RTE_CACHE_LINE_SIZE)

#define RTE_ALIGN_FLOOR(val, align) \
	(typeof(val))((val) & (~((typeof(val))((align) - 1))))

#define RTE_ALIGN_CEIL(val, align) \
	RTE_ALIGN_FLOOR(((val) + ((typeof(val)) (align) - 1)), align)

#define RTE_ALIGN(val, align) RTE_ALIGN_CEIL(val, align)

#define __rte_always_inline inline __attribute__((always_inline))

#define RTE_SET_USED(x) (void)(x)

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif /* likely */

#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif /* unlikely */

#include <emmintrin.h>
static inline void rte_pause(void)
{
	_mm_pause();
}

#ifdef __cplusplus
}
#endif

#endif