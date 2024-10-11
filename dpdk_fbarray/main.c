#include <stdio.h> 
#include "rte_pause.h"
#include "rte_atomic.h"

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
   
	return argc;
}
