#ifndef _mm_pool_h_
#define _mm_pool_h_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  mem_pools_s
{
    int m_block_size;
    int m_free_count;
    int m_block_count;
    char *m_free_ptr;
    char *m_mem;
} mem_pools_t;

int mem_create(mem_pools_t * mem, int block_count, int block_size)
{
    if(mem == nullptr)
    {
        return -1; 
    }
    mem->m_free_count = mem->m_block_count= block_count;
    mem->m_block_size = block_size;
    mem->m_mem =  (char *)malloc(mem->m_free_count * block_size);
    mem->m_free_ptr = mem->m_mem;
    char * ptr = mem->m_mem;
    for(int i = 0; i < block_count; ++i)
    {
        *((char **)ptr) = ptr + block_size;
        ptr += block_size;
    }
    *((char **)ptr) = nullptr;
    return 0;
}

void mem_destory(mem_pools_t * mem)
{
    if(mem->m_mem)
    {
        free(mem->m_mem);
    }
}

void * mem_alloc(mem_pools_t * mem)
{
    if(mem->m_free_count == 0)
    {
        return nullptr;
    }
    char * ptr = mem->m_free_ptr;
    mem->m_free_ptr = *((char **)ptr);
    --(mem->m_free_count);
    return ptr;
}

void mem_free(mem_pools_t * mem, void * ptr)
{
    if(ptr < mem->m_mem || ptr >= (mem->m_mem + mem->m_free_count * mem->m_block_size))
    {
        return;
    }
    *((char **)ptr) = mem->m_free_ptr;
    mem->m_free_ptr = (char *)ptr;
    mem->m_free_count++; 
}

#ifdef __cplusplus
}
#endif

#endif