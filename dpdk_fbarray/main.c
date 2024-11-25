#include <stdio.h> 
#include "rte_pause.h"
#include "rte_atomic.h"
#include "rte_filesystem.h"
#include <unistd.h>
#include "rte_fbarray.h"

void print_fbarray_info(const struct rte_fbarray *arr)
{
    if (arr == NULL) {
        printf("Error: Null pointer passed to print_fbarray_info.\n");
        return;
    }

    printf("----- fbarray Info -----\n");
    printf("Name: %s\n", arr->name);
    printf("Count (entries stored): %u\n", arr->count);
    printf("Length (current length of the array): %u\n", arr->len);
    printf("Element Size: %u bytes\n", arr->elt_sz);
    printf("Data Pointer: %p\n", arr->data);
    
    // Print lock address
    printf("Read-Write Lock (address): %p\n", &arr->rwlock);

    printf("------------------------\n");
}

int main(int argc, char *argv[])
{
	printf("%s\n", argv[0]);
	
	rte_pause();

	// 定义并初始化 rte_atomic16_t 类型的变量
    rte_atomic16_t i16;
    rte_atomic16_init(&i16);
    rte_atomic16_add(&i16, 5);

    // 设置比较和交换操作的期望值和新值
    volatile uint16_t ui16 = 0;
    uint16_t expected = 0;
    uint16_t new_value = 1;

    // 进行原子比较并交换操作
    int ret = rte_atomic16_cmpset(&ui16, expected, new_value);
    printf("Atomic16 cmpset result: %d (expected: %d, new value: %d)\n", ret, expected, new_value);

    // 定义并初始化 rte_atomic32_t 类型的变量
    rte_atomic32_t i32;
    rte_atomic32_init(&i32);

    // 进行原子加法操作（32位）
    rte_atomic32_add(&i32, 5);
    printf("Atomic32 value after addition: %d\n", rte_atomic32_read(&i32));

    // 定义并初始化 rte_atomic64_t 类型的变量
    rte_atomic64_t i64;
    rte_atomic64_init(&i64);

    // 进行原子加法操作（64位）
    rte_atomic64_add(&i64, 10);
    printf("Atomic64 value after addition: %ld\n", rte_atomic64_read(&i64));
    eal_create_runtime_dir();

    printf(" pid[%d] runtimedir[%s]\n", getpid(), eal_get_runtime_dir());
    {
        struct rte_fbarray array; // 声明一个 fbarray 结构体
        #define ELEMENT_SIZE sizeof(int)
        #define ARRAY_LENGTH 17

        // 初始化 fbarray 数组
        int ret = rte_fbarray_init(&array, "lyj_fbarray",  ARRAY_LENGTH, ELEMENT_SIZE);
        if (ret != 0) {
            printf("Error initializing fbarray: %d\n", ret);
            return -1;
        }
        print_fbarray_info(&array);

        // 填充数组元素
        for (unsigned int i = 0; i < ARRAY_LENGTH / 2; i++)
        {
            int *element = (int *)rte_fbarray_get(&array, i);
            if (element == NULL)
            {
                fprintf(stderr, "Failed to get element at index %d\n", i);
                continue;
            }
            *element = i * 10; // 示例数据
            if (rte_fbarray_set_used(&array, i) != 0) {
                fprintf(stderr, "Failed to set element at index %d as used\n", i);
            }
        }
        printf("Array populated with sample data.\n");

        // 读取并打印数组元素
        printf("Array contents:\n");
        for (unsigned int i = 0; i < ARRAY_LENGTH ; i++)
        {
          if (rte_fbarray_is_used(&array, i))
            {
                int *element = (int *)rte_fbarray_get(&array, i);
                if (element != NULL)
                {
                    printf("Index %d: %d\n", i, *element);
                }
            }
        }
        print_fbarray_info(&array);

        // 查找第一个空闲元素
        int free_idx = rte_fbarray_find_next_free(&array, 0);
        if (free_idx >= 0)
        {
            printf("First free index: %d\n", free_idx);
        }
        else
        {
            printf("No free elements found.\n");
        }

        // 查找第一个已使用的元素
        int used_idx = rte_fbarray_find_next_used(&array, 0);
        if (used_idx >= 0)
        {
            printf("First used index: %d\n", used_idx);
        }
        else
        {
            printf("No used elements found.\n");
        }
        while(1);

        // 输出数组的元数据
        // printf("Array metadata:\n");
        // rte_fbarray_dump_metadata(&array, stdout);

        // 销毁数组
        if (rte_fbarray_destroy(&array) != 0) {
            fprintf(stderr, "Failed to destroy array\n");
            return EXIT_FAILURE;
        }
        printf("Array destroyed.\n");

    }

	return argc;
}
