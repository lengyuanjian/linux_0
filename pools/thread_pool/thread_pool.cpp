#include "thread_pool.h"
#include <pthread.h> 
#include <stdlib.h>
// pthread_addr_t attr
//对称试资源接口设计

//资源创建回滚试代码  业务逻辑用防御试代码
 
struct sh_task
{
    void * next;
    sh_td_func fun;
    void * args;
};

struct sh_task_queue
{
    void * head;
    void ** tail;
    int block;
    pthread_spinlock_t spinlock;
};

sh_task_queue *  sh_create_task_queue()
{
    sh_task_queue * queue = (sh_task_queue *)malloc(sizeof(sh_task_queue));
    if(queue)
    {   
        int ret = pthread_spin_init(&(queue->spinlock), 0); //不可跨进程
        if(ret == 0)
        {
            queue->head = nullptr;
            queue->tail = &(queue->head);
            queue->block = 1;
            return queue;
        }
        free(queue);
    }
    return nullptr;
}

void push_task(sh_task_queue * queue, sh_td_func fun, void *args)
{
    sh_task * task = (sh_task *)malloc(sizeof(sh_task));
    if(task == nullptr)
    {
        return;
    }
    task->args = args;
    task->fun = fun;

    void **link = (void **)task;
    *link = nullptr;
    
    pthread_spin_lock(&(queue->spinlock));
    *(queue->tail) = link;
    queue->tail = link;
    pthread_spin_unlock(&(queue->spinlock));
}

sh_task * pop_task(sh_task_queue * queue)
{   
    sh_task * task = nullptr;
    pthread_spin_lock(&(queue->spinlock));
    if(queue->head != nullptr)
    {
        task = (sh_task *)(queue->head);
        void **link = (void**)task;
        queue->head = *link;

        if(queue->head == nullptr) 
        {
            queue->tail = &queue->head;
        }
    }
    pthread_spin_unlock(&(queue->spinlock)); 
    return task;
}

void sh_destory_task_queue(sh_task_queue * queue)
{
    // 循环释放所有任务
    sh_task * task;
    while ((task = pop_task(queue)) != nullptr)
    {
        free(task);
    }
    pthread_spin_destroy(&(queue->spinlock));
    free(queue);
}

struct sh_thread_pool
{
    sh_task_queue *task_queue;
    int quit;
    int th_count;
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void * thread_work(void *args)
{
    sh_thread_pool * pool = (sh_thread_pool *)args;
    while(pool->quit == 0)
    {
        sh_task * task = pop_task(pool->task_queue);
        if(task)
        {
            (task->fun)(task->args);
            free(task);
        }
        else
        {
            pthread_mutex_lock(&(pool->mutex));
            if(pool->quit == 1)
            {
                pthread_mutex_unlock(&(pool->mutex));
                break;
            }
            pthread_cond_wait(&(pool->cond), &(pool->mutex));
            pthread_mutex_unlock(&(pool->mutex));
        }
    }
    return args;
}
int init_threads(sh_thread_pool *pool, int count)
{
    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);
    if(ret == 0)
    {
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * count);
        if(pool->threads)
        {
            int i = 0;
            for(; i < count; ++i)
            {
                pthread_create(pool->threads + i, &attr, thread_work, pool);
            }
            pool->th_count = i;
            pthread_attr_destroy(&attr);
            if(i == count)
            {
                return 0;
            }
            // 销毁所有线程
            free(pool->threads);
        }
        ret = -1;
    }
    return ret;
}
sh_thread_pool *sh_create_thread_pool(int count)
{
    if(count <= 0)
    {
        return nullptr;
    }
    sh_thread_pool * pool = (sh_thread_pool*)malloc(sizeof(sh_thread_pool));
    if(pool)
    {
        sh_task_queue * queue = sh_create_task_queue();
        if(queue)
        {
            pool->task_queue = queue;
            int ret = pthread_mutex_init(&(pool->mutex), nullptr);
            if(ret == 0)
            {
                ret = pthread_cond_init(&(pool->cond), nullptr); //不可跨进程
                if(ret == 0)
                {
                    pool->quit = 0;
                    if(init_threads(pool, count) == 0)
                    {
                        return pool;
                    }
                    pthread_cond_destroy(&(pool->cond));
                }
                pthread_mutex_destroy(&(pool->mutex));
            } 
            sh_destory_task_queue(pool->task_queue);
        }
        free(pool);
    }
    return nullptr;
}

void sh_destory_thread_pool(sh_thread_pool * pool)
{
    pthread_mutex_lock(&(pool->mutex));
    pool->quit = 1;
    pthread_mutex_unlock(&(pool->mutex));
    pthread_cond_broadcast(&(pool->cond));
    for(int i = 0; i < pool->th_count; ++i)
    {
        pthread_join(pool->threads[i], nullptr);
    }
    pthread_cond_destroy(&(pool->cond)); 
    pthread_mutex_destroy(&(pool->mutex)); 
    sh_destory_task_queue(pool->task_queue);
    free(pool->threads);
    free(pool);
}

void sh_waitdone_thread_pool(sh_thread_pool * pool)
{
    (void)pool;
}

int sh_push_thread_pool(sh_thread_pool * pool, sh_td_func fun, void * args)
{
    push_task(pool->task_queue,fun,args);
    pthread_cond_signal(&(pool->cond));
    return 0;
}