#include "spdk/env.h"
#include "spdk/thread.h"
#include "spdk/log.h"
#include <stdio.h>
#include <spdk/init.h>
#include <spdk/bdev.h>

// 任务的回调函数

bool waiter(struct spdk_thread *thread, spdk_msg_fn fun, void *arg,  int * finished);

struct context_t
{
    struct spdk_bs_dev *bs_dev;
    int finish;
} context={NULL};

bool waiter(struct spdk_thread *thread, spdk_msg_fn fun, void *arg,  int * finished)
{
    static const int WAITER_MAX_TIME = 100000;
    spdk_thread_send_msg(thread, fun, arg);
    int count = 0;
    do 
    {
        spdk_thread_poll(thread, 0, 0);
        ++count;
    }while((*finished == -1) && count < WAITER_MAX_TIME);

    if(*finished == 0)
    {
        return true;
    }
    
    return false;
}

void fun_json_load_cb(int rc, void *ctx)
{
    if (rc == 0) 
    {
        int *finish = ctx;
        *finish = 0;
    } 
    else 
    {
        int *finish = ctx;
        *finish = 1;
        printf("加载失败，错误码: %d\n", rc);
    }
}

void fun_json_load(void *arg)
{
    spdk_subsystem_init_from_json_config("/home/spdk/examples/blob/hello_world_bak/conf.json",SPDK_DEFAULT_RPC_ADDR,fun_json_load_cb, arg, true);
}

static void base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev, void *event_ctx)
{
	SPDK_WARNLOG("Unsupported bdev event: type %d\n", (int)type);
	SPDK_NOTICELOG("Unsupported bdev event: type %d\n", (int)type);
}
static void bs_init_complete(void *arg, struct spdk_blob_store *bs, int bserrno)
{
	struct context_t * p_context = (struct context_t *)arg;

        printf("bs_init_complete[%d]\n", bserrno);
    if (bserrno == 0) 
    {
         
        p_context->finish = 0;
    } 
    else 
    {
        p_context->finish = 1;
        printf("加载失败，错误码: %d\n", bserrno);
    }
	SPDK_NOTICELOG("entry\n");
	if (bserrno) 
	{
		printf("hello_context, Error initing the blobstore[%d]\n", bserrno);
		return;
	}
    else
    {
    }

	//hello_context->bs = bs;
	//SPDK_NOTICELOG("blobstore: %p\n", hello_context->bs);
	/*
	 * We will use the io_unit size in allocating buffers, etc., later
	 * so we'll just save it in out context buffer here.
	 */
	printf("spdk_bs_get_io_unit_size%u\n",spdk_bs_get_io_unit_size(bs));

	//create_blob(hello_context);
}

void fun_bs_init(void *arg)
{
    struct context_t * p_context = (struct context_t *)arg;
    spdk_bs_init(p_context->bs_dev, NULL, bs_init_complete, arg);
}

// 主程序入口
int main(int argc, char **argv)
{
    // 初始化SPDK环境
    struct spdk_env_opts opts;
    spdk_env_opts_init(&opts);
    
	opts.name = "spdk_thread_example";
    opts.opts_size = sizeof(struct spdk_env_opts);
	if (spdk_env_init(&opts) < 0) {
        SPDK_ERRLOG("Unable to initialize SPDK env\n");
        return -1;
    }
    spdk_log_set_print_level(SPDK_LOG_NOTICE);
	spdk_log_set_level(SPDK_LOG_NOTICE);
	spdk_log_open(NULL);

    pthread_t thread_id = pthread_self();
    printf("POSIX Thread ID: %lu\n", (unsigned long)thread_id);

    // 创建SPDK线程
    spdk_thread_lib_init(NULL, 0);
    struct spdk_thread *thread = spdk_thread_create("example_thread", NULL);
    if (!thread) 
    {
          SPDK_ERRLOG("Unable to create SPDK thread\n");
          spdk_env_fini();
          return -1;
    }
    spdk_set_thread(thread);
    int finish = -1;
    bool ret = waiter(thread, fun_json_load, &finish, &finish);
    if(ret)
    {
        printf("加载文件成功\n");
    }
    else
    {
        printf("加载文件失败\n");
    }
 
	struct spdk_bs_dev *bs_dev = NULL;
	int rc = spdk_bdev_create_bs_dev_ext("Malloc0", base_bdev_event_cb, NULL, &bs_dev);
	if (rc != 0) 
    {
		SPDK_ERRLOG("Could not create blob bdev, %s!!\n", spdk_strerror(-rc));
		spdk_app_stop(-1);
		return 0;
	}
    else
    {
        printf("create bs dev ext\n");
    }
    context.bs_dev = bs_dev;
    context.finish = -1;
    ret = waiter(thread, fun_bs_init, &context, &(context.finish));
    if(ret)
    {
        printf("fun_bs_init ok\n");
    }
    else
    {
        printf("fun_bs_init failed\n");
    }


	while(true)
	{
        printf("spdk:");
        fflush(stdout);
		char cmd[1024]={0};
		if(NULL != fgets(cmd, 1024,stdin))
        {
            int len = strlen(cmd);
            if(len > 0)
            {
                cmd[len - 1] = '\0';
                printf("cmd[%s]\n", cmd);
            }
            if(cmd[0] == 'q' || cmd[0] == 'Q')
            {
                break;
            }
            //spdk_thread_poll(thread, 0, 0);
        }
	}
    spdk_thread_exit(thread);
    spdk_thread_destroy(thread);

    //清理SPDK环境
    spdk_env_fini();

    return 0;
}
