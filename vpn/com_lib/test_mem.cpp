#include <stdio.h>
#include <assert.h>
#include "mem_lib/mem_pool.h"

// 测试创建和销毁内存池
void test_create_destroy() {
    printf("=== Testing create and destroy ===\n");
    mem_pool_t pool;
    int ret = create_mem_pool(&pool, 64, 10, 5);
    assert(ret == 0);
    assert(pool.block_size == 64);
    assert(pool.block_total == 10);
    assert(pool.free_count == 10);
    assert(pool.free_ptr != NULL);
    
    destroy_mem_pool(&pool);
    assert(pool.mm == NULL);
    assert(pool.free_ptr == NULL);
    printf("Test create and destroy passed!\n\n");
}

// 测试内存分配和释放
void test_alloc_free() {
    printf("=== Testing allocation and freeing ===\n");
    mem_pool_t pool;
    create_mem_pool(&pool, 64, 3, 2); // 初始3块，自动增长2块
    
    // 分配3块内存
    void* ptr1 = malloc_mem_pool(&pool);
    void* ptr2 = malloc_mem_pool(&pool);
    void* ptr3 = malloc_mem_pool(&pool);
    
    assert(ptr1 != NULL);
    assert(ptr2 != NULL);
    assert(ptr3 != NULL);
    assert(pool.free_count == 0);
    assert(pool.free_ptr == NULL);
    
    // 再分配应该触发自动增长
    void* ptr4 = malloc_mem_pool(&pool);
    assert(ptr4 != NULL);
    assert(pool.free_count == 1); // 新增长2块，用了1块
    assert(pool.block_total == 5); // 3 + 2
    
    // 释放内存
    free_mem_pool(&pool, ptr2);
    assert(pool.free_count == 2);
    
    free_mem_pool(&pool, ptr1);
    assert(pool.free_count == 3);
    
    // 测试释放无效指针
    int dummy;
    free_mem_pool(&pool, &dummy); // 应该打印错误日志
    
    destroy_mem_pool(&pool);
    printf("Test allocation and freeing passed!\n\n");
}

// 测试边界情况
void test_edge_cases() {
    printf("=== Testing edge cases ===\n");
    // 测试无效参数
    mem_pool_t pool;
    int ret = create_mem_pool(NULL, 64, 10, 5);
    assert(ret == -1);
    
    ret = create_mem_pool(&pool, 0, 10, 5);
    assert(ret == -1);
    
    ret = create_mem_pool(&pool, 64, 0, 5);
    assert(ret == -1);
    
    ret = create_mem_pool(&pool, 64, 10, 0);
    assert(ret != -1);
    
    // 测试内存耗尽
    mem_pool_t pool2;
    create_mem_pool(&pool2, 64, 1, 0); // 无自动增长
    void* p1 = malloc_mem_pool(&pool2);
    assert(p1 != NULL);
    
    void* p2 = malloc_mem_pool(&pool2);
    assert(p2 == NULL); // 应该返回NULL
    
    destroy_mem_pool(&pool2);
    printf("Test edge cases passed!\n\n");
}

#include <stdlib.h> // 用于 rand() 和 srand()
#include <time.h>  // 用于 time()

// 交换两个指针的辅助函数
void swap_pointers(void** a, void** b) {
    void* temp = *a;
    *a = *b;
    *b = temp;
}

// 随机打乱指针数组
void shuffle_pointers(void* pointers[], int size) {
    srand(time(NULL)); // 初始化随机种子
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1); // 随机选择索引
        swap_pointers(&pointers[i], &pointers[j]);
    }
}
#include <cstring> // 用于 memset()
// 性能测试
void test_performance() {
    printf("=== Testing performance ===\n");
    mem_pool_t pool;
    create_mem_pool(&pool, 64, 1000, 200);
    
    // 分配1000次
    void* pointers[2000];
    int len = 900;
    for (int i = 0; i < len; i++) {
        pointers[i] = malloc_mem_pool(&pool);
        assert(pointers[i] != NULL);
    }
    for(int i = 0; i < 5000; ++i)
    {
        shuffle_pointers(pointers, len);
        
        // 随机释放一半
        for (int i = 0; i < len / 2; i++) {
            free_mem_pool(&pool, pointers[i]);
        }
        
        // 再分配500次
        for (int i = 0; i < len / 2; i++) {
            pointers[i] = malloc_mem_pool(&pool);
            memset(pointers[i], 0, pool.block_size);
            assert(pointers[i] != NULL);
        }
        if(len < 2000)
        {
            int pos = len;
            len += 100;
            if(len > 2000)
            {
                len = 2000;
            }
            for (; pos < len; pos++) {
                pointers[pos] = malloc_mem_pool(&pool);
                assert(pointers[pos] != NULL);
            }
        }
    }
    
    destroy_mem_pool(&pool);
    printf("Performance test completed!\n\n");
}

int main_test() {
    test_create_destroy();
    test_alloc_free();
    test_edge_cases();
    test_performance();
    
    printf("All tests passed successfully!\n");
    return 0;
}