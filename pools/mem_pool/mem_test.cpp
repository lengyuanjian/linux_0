#include <iostream>
 #include "mm_pool.h"


int main(int argc, char * argv[]) 
{
    
    const char * ip = argv[1];
    std::cout<<ip <<"\n";

    mem_pools_t pool;

    // Create a memory pool with 10 blocks, each of size 32 bytes
    if (mem_create(&pool, 1024, 16) != 0) {
        printf("Failed to create memory pool\n");
        return -1;
    }

    printf("Memory pool created successfully\n");

    // Allocate some memory blocks from the pool
    void *ptr1 = mem_alloc(&pool);
    void *ptr2 = mem_alloc(&pool);
    void *ptr3 = mem_alloc(&pool);

    if (!ptr1 || !ptr2 || !ptr3) {
        printf("Failed to allocate memory from the pool\n");
        mem_destory(&pool);
        return -1;
    }
    printf("ptr1[%p]\n",ptr1);
    printf("ptr2[%p]\n",ptr2);
    printf("ptr3[%p]\n",ptr3);
    
    printf("Allocated memory blocks successfully\n");

    // Free one memory block back to the pool
    mem_free(&pool, ptr2);
    printf("Freed one memory block\n");

    // Allocate another memory block from the pool
    void *ptr4 = mem_alloc(&pool);
    if (!ptr4) {
        printf("Failed to allocate memory from the pool\n");
        mem_destory(&pool);
        return -1;
    }
    printf("ptr4[%p]\n",ptr4);

    void *ptr5 = mem_alloc(&pool);
    if (!ptr5) {
        printf("Failed to allocate memory from the pool\n");
        mem_destory(&pool);
        return -1;
    }
    printf("ptr5[%p]\n",ptr5);

    printf("Allocated another memory block successfully\n");

    // Destroy the memory pool
    mem_destory(&pool);
    printf("Memory pool destroyed successfully\n");
   
    return argc;
}
