#include <thread>
#include <atomic>
#include <cassert>
#include <stdlib.h>
#include <stdio.h>
/*
代码段 (Text Segment)
    存储程序的可执行代码。
    通常为只读，防止代码被修改。
数据段 (Data Segment)
    存储已初始化的全局变量和静态变量。
BSS段 (BSS Segment)
    存储未初始化的全局变量和静态变量。
堆 (Heap)
    地址从数据段的末尾向高地址方向增长。
    用于动态内存分配（例如，使用malloc函数）。
栈 (Stack)

*/

// 定义一些全局变量来表示数据段和BSS段
int initialized_data = 42;  // 数据段
int uninitialized_data;     // BSS段

// 获取代码段的起始地址和结束地址
extern char _etext, _edata, _end;

#define CODE_SEGMENT_START ((uintptr_t)&_etext)
#define CODE_SEGMENT_END   ((uintptr_t)&_edata)
#define DATA_SEGMENT_START ((uintptr_t)&initialized_data)
#define BSS_SEGMENT_START  ((uintptr_t)&uninitialized_data)

// 声明函数来打印内存布局
void print_memory_layout(void *heap_start, void *heap_end, void *stack_start);

// 主函数
int main() {
    // 分配一些堆内存以获取堆的起始地址和结束地址
    void *heap_start = malloc(1);
    void *heap_end = malloc(1);

    // 声明一个局部变量以获取栈的起始地址
    int stack_start_var;
    void *stack_start = &stack_start_var;

    print_memory_layout(heap_start, heap_end, stack_start);

    // 释放堆内存
    free(heap_start);
    free(heap_end);

    return 0;
}

// 获取栈的结束地址（通过递归函数调用来模拟栈增长）
void get_stack_end(void **stack_end) {
    int stack_end_var;
    *stack_end = &stack_end_var;
}

void print_memory_layout(void *heap_start, void *heap_end, void *stack_start) {
    void *stack_end;
    get_stack_end(&stack_end);

    // 打印各段的地址范围
    printf("Code Segment: 0x%lx - 0x%lx\n", CODE_SEGMENT_START, CODE_SEGMENT_END);
    printf("Data Segment: 0x%lx\n", DATA_SEGMENT_START);
    printf("BSS Segment:  0x%lx\n", BSS_SEGMENT_START);
    printf("Heap:         0x%lx - 0x%lx\n", (uintptr_t)heap_start, (uintptr_t)heap_end);
    printf("Stack:        0x%lx - 0x%lx\n", (uintptr_t)stack_end, (uintptr_t)stack_start);
}

/*
地址空间划分
用户空间 (User Space)

地址范围：0x0000000000000000 到 0x00007FFF FFFFFFFF
大小：128TB
用户空间是应用程序运行的区域。每个用户进程拥有独立的用户空间地址区域，用户空间地址在不同的进程中可以相同，但它们映射到不同的物理内存区域。
内核空间 (Kernel Space)

地址范围：0xFFFF8000 00000000 到 0xFFFFFFFFFFFFFFFF
大小：128TB
内核空间是操作系统内核运行的区域，所有进程共享同一个内核空间地址区域。内核空间地址只在内核模式下访问，用户进程不能直接访问内核空间地址。
用户空间详细划分
在用户空间内，地址空间进一步划分为不同的区域：

代码段 (Text Segment)

存储程序的可执行代码。
通常为只读，防止代码被修改。
数据段 (Data Segment)

存储已初始化的全局变量和静态变量。
BSS段 (BSS Segment)

存储未初始化的全局变量和静态变量。
堆 (Heap)

地址从数据段的末尾向高地址方向增长。
用于动态内存分配（例如，使用malloc函数）。
栈 (Stack)

地址从高地址向低地址方向增长。
用于存储函数调用的返回地址、本地变量等。
*/