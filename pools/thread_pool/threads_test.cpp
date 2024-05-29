#include <iostream>
#include <stdio.h>
#include <unistd.h>
// workflow c++ 
#include "thread_pool.h"
#include <pthread.h>



// 定义一个任务函数
void task_function(void *arg) 
{
    int *task_id = (int *)arg;
    pthread_t thread_id = pthread_self(); // 获取当前线程的 ID
    printf("Task %d is running on thread %lu\n", *task_id, thread_id);
    sleep(1); // 模拟任务执行时间
    printf("Task %d is done on thread %lu\n", *task_id, thread_id);
    free(task_id);
}

int main(int argc, char * argv[]) 
{
    
   const char * ip = argv[1];
   std::cout<< ip <<"\n";
   sh_thread_pool * pool =  sh_create_thread_pool(3);
   if(pool == nullptr)
   {
        std::cout<<"线程池创建失败";
        return 0;
   }
 

    // 推送任务到线程池
    for (int i = 0; i < 10; ++i) {
        int *task_id = (int *)malloc(sizeof(int)); // 为每个任务分配一个任务 ID
        *task_id = i;
        sh_push_thread_pool(pool, task_function, task_id); // 推送任务到线程池
    }
    sleep(10);
    // 等待所有任务完成
    sh_waitdone_thread_pool(pool);

    // 销毁线程池
    sh_destory_thread_pool(pool);

    return argc;
}

