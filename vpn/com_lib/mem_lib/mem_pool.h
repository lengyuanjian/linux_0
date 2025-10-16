#ifndef __MEM_POOL_H_
#define __MEM_POOL_H_

#include <stdlib.h>
#include "../err_log/lib_log.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_page_s
{
    char *beg;
    char *end;
    mem_page_s *next;
}mem_page_t;

typedef struct mem_pool_s
{
    int block_size;
    int block_total;
    int auto_increment;
    int free_count;
    char *free_ptr;
    mem_page_t *mm;
}mem_pool_t;

inline mem_page_t *malloc_mem_page(int block_size, int block_count)
{
    int mem_page_size = block_size * block_count + sizeof(mem_page_t);
    void *ptr = malloc(mem_page_size);
    if(ptr == NULL)
    { 
        ERROR_LOG("malloc mem_page failed, page size = %d, block size = %d, block count = %d", mem_page_size, block_size, block_count);
        return NULL;
    }

    mem_page_t *page = (mem_page_t *)(((char*)ptr) + block_size * block_count);
    page->beg = (char *)ptr;
    page->end = (char *)page;
    page->next = NULL;

    char *ptr1 = page->beg;
    for(int i = 0; i < block_count - 1; ++i)
    {
        *(char **)ptr1 = ptr1 + block_size;
        ptr1 += block_size;
    }
    *(char **)ptr1 = NULL;
    return page;
}

inline int create_mem_pool(mem_pool_t *pool, int block_size, int block_count, int auto_increment)
{
    if(pool == NULL) return -1;
    if (block_size <= 0 || block_count <= 0)
        return -1;

    pool->block_size = block_size;
    pool->auto_increment = auto_increment;
    pool->free_count = block_count;
    pool->block_total = block_count;
    
    mem_page_t *page = malloc_mem_page(block_size, block_count);
    if(page == NULL) return -1;
    pool->mm = page;
    pool->free_ptr = page->beg;

    return 0;
}
inline void destroy_mem_pool(mem_pool_t *pool)
{
    if(pool == NULL) return;
    mem_page_t *page = pool->mm;
    while(page)
    {
        mem_page_t *next = page->next;
        free(page->beg);
        page = next;
    }
    pool->mm = NULL;
    pool->free_ptr = NULL;
    pool->free_count = 0;
    pool->block_size = 0;
    pool->auto_increment = 0;
}

inline void *malloc_mem_pool(mem_pool_t *pool)
{
    if(pool == NULL) return NULL;
    if(pool->free_ptr == NULL)
    {
        if(pool->auto_increment <= 0) return NULL;
        mem_page_t *page = malloc_mem_page(pool->block_size, pool->auto_increment);
        if(page == NULL) return NULL;
        page->next = pool->mm;
        pool->mm = page;
        pool->free_ptr = page->beg;
        pool->free_count += pool->auto_increment;
        pool->block_total += pool->auto_increment;
        DEBUG_LOG("malloc new page, block size = %d, block count = %d", pool->block_size, pool->auto_increment);
    }
    char *ptr = pool->free_ptr;
    pool->free_ptr = *(char **)ptr;
    --pool->free_count;
    return ptr;
}

inline void free_mem_pool(mem_pool_t *pool, void *ptr)
{
    if(pool == NULL || ptr == NULL) return;

    mem_page_t *page = pool->mm;
    int is_find = 0;
    while(page)
    {
        if(ptr >= page->beg && ptr < page->end)
        {
            is_find = 1;
            int offset = (char *)ptr - page->beg;
            if(offset % pool->block_size == 0)
            {
                *(char **)ptr = pool->free_ptr;
                pool->free_ptr = (char *)ptr;
                ++pool->free_count;
            }
            else 
            {
                ERROR_LOG("free mem_pool failed, ptr = %p, offset = %d, beg = %p, end =%p", ptr, offset, page->beg, page->end);
            }
            break;
        }
        page = page->next;
    }
    if(!is_find)
    {
        ERROR_LOG("free mem_pool failed, not find ptr = %p, not in mem_pool", ptr);
    }
}

#if __cplusplus
}
#endif
#endif