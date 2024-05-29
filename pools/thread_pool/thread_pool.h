#ifndef thread_pool
#define thread_pool


typedef void (*sh_td_func)(void *);

struct sh_thread_pool;

#ifdef __cplusplus
extern "C" {
#endif

sh_thread_pool * sh_create_thread_pool(int count);

void sh_destory_thread_pool(sh_thread_pool * pool);

int sh_push_thread_pool(sh_thread_pool * pool, sh_td_func fun, void * args);

void sh_waitdone_thread_pool(sh_thread_pool * pool);

#ifdef __cplusplus
}
#endif

#endif 