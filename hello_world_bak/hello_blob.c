#include "spdk/env.h"
#include "spdk/thread.h"
#include "spdk/event.h"
#include "spdk/blob_bdev.h"
#include "spdk/blob.h"
#include "spdk/log.h"
#include <stdio.h>
#include <spdk/init.h>
#include <spdk/bdev.h>
// #include "limit.h"
// 任务的回调函数

bool waiter(struct spdk_thread *thread, spdk_msg_fn fun, void *arg,  int * finished);

typedef struct disk_context
{
    const char *                m_p_json_config_file;
    struct spdk_thread *        m_p_thread;
    struct spdk_bs_dev *        m_p_bs_dev;
    struct spdk_blob_store*     m_p_bs;
	uint64_t                    m_io_unit_size;
    int                         m_finish;
}disk_context_t;

typedef struct file_context
{ 
	spdk_blob_id                m_blobid;
    struct spdk_blob_store*     m_p_bs;
	struct spdk_blob*           m_p_blob;
	struct spdk_io_channel *    m_p_channel;
	uint8_t *                   m_p_read_buff;
	uint8_t *                   m_p_write_buff;
    uint64_t                    m_pos;
    uint64_t                    m_size;
	int                         m_rc;
    int                         m_finish;
} file_context_t;

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

void fun_json_load_cb(int rc, void *arg)
{
    disk_context_t * p_disk = (disk_context_t *)arg;
    if (rc == 0) 
    {
        p_disk->m_finish = 0;
    } 
    else 
    {
        p_disk->m_finish = 1;
        printf("加载失败，错误码: %d\n", rc);
    }
}

void fun_json_load(void *arg)
{
    disk_context_t * p_disk = (disk_context_t *)arg;
    spdk_subsystem_init_from_json_config(p_disk->m_p_json_config_file,SPDK_DEFAULT_RPC_ADDR,fun_json_load_cb, arg, true);
}

static void base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev, void *event_ctx)
{
	SPDK_WARNLOG("Unsupported bdev event: type %d\n", (int)type);
	SPDK_NOTICELOG("Unsupported bdev event: type %d\n", (int)type);
}

static void bs_init_complete(void *arg, struct spdk_blob_store *bs, int bserrno)
{
	disk_context_t * p_disk = (disk_context_t *)arg;
    printf("bs_init_complete[%d]\n", bserrno);
    if (bserrno == 0) 
    {
        p_disk->m_p_bs = bs;
        SPDK_NOTICELOG("blobstore: %p\n", bs); 
        p_disk->m_io_unit_size = spdk_bs_get_io_unit_size(bs);
        printf("spdk_bs_get_io_unit_size%lu\n",p_disk->m_io_unit_size); 
        p_disk->m_finish = 0;
    } 
    else 
    {
        printf("加载失败，错误码: %d\n", bserrno);
		printf("hello_context, Error initing the blobstore[%d]\n", bserrno);
        p_disk->m_finish = 1;
    } 
}

void fun_bs_init(void *arg)
{
    disk_context_t * p_disk = (disk_context_t *)arg;
    spdk_bs_init(p_disk->m_p_bs_dev, NULL, bs_init_complete, arg);
}

void blob_create_complete(void *arg, spdk_blob_id blobid, int bserrno)
{
	file_context_t * p_file = (file_context_t *)arg;
	if (bserrno) 
    {
		// unload_bs(hello_context, "Error in blob create callback",
		// 	  bserrno);
        p_file->m_finish = 1;
		return;
	}
    else
    {
        p_file->m_blobid = blobid;
	    SPDK_NOTICELOG("new blob id %" PRIu64 "\n", blobid);
        p_file->m_finish = 0;
    }
}

void fun_create_blob(void *arg)
{
    file_context_t * p_file = (file_context_t *)arg;
	spdk_bs_create_blob(p_file->m_p_bs, blob_create_complete, p_file);
}
void open_complete(void *arg, struct spdk_blob *blob, int bserrno)
{
    file_context_t * p_file = (file_context_t *)arg;
    if (bserrno) 
    {
		// unload_bs(hello_context, "Error in open completion",
		// 	  bserrno);
        p_file->m_finish = 1;
		return;
	}
    else
    {
        p_file->m_p_blob = blob;
        p_file->m_finish = 0;
    }
}
void fun_open_blob(void *arg)
{
    file_context_t * p_file = (file_context_t *)arg;
	spdk_bs_open_blob(p_file->m_p_bs, p_file->m_blobid, open_complete, p_file);
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

    disk_context_t * p_disk = (disk_context_t *)calloc(1, sizeof(disk_context_t));
    {
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
        p_disk->m_p_thread = thread;
        spdk_set_thread(thread);
    }
    {
        p_disk->m_p_json_config_file = "/home/spdk/examples/blob/hello_world_bak/conf.json";
        p_disk->m_finish = -1; 
        bool ret = waiter(p_disk->m_p_thread, fun_json_load, p_disk, &(p_disk->m_finish));
        if(ret)
        {
            printf("加载文件成功\n");
        }
        else
        {
            printf("加载文件失败\n");
        }
    
        int rc = spdk_bdev_create_bs_dev_ext("Malloc0", base_bdev_event_cb, NULL, &(p_disk->m_p_bs_dev));
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
        p_disk->m_finish = -1; 
        ret = waiter(p_disk->m_p_thread, fun_bs_init,p_disk, &(p_disk->m_finish));
        if(ret)
        {
            printf("fun_bs_init ok\n");
        }
        else
        {
            printf("fun_bs_init failed\n");
        }
    }
    file_context_t * p_file = (file_context_t *)calloc(1, sizeof(file_context_t));
    p_file->m_p_bs = p_disk->m_p_bs;
    // 创建 blob
    {
        p_file->m_finish = -1; 
        bool ret = waiter(p_disk->m_p_thread, fun_create_blob, p_file, &(p_file->m_finish));
        if(ret)
        {
            printf("blob creat ok\n");
        }
        else
        {
            printf("blob creat failed\n");
        }
    }
    // 打开 blob
    {
        p_file->m_finish = -1; 
        bool ret = waiter(p_disk->m_p_thread, fun_open_blob, p_file, &(p_file->m_finish));
        if(ret)
        {
            printf("blob open ok\n");
        }
        else
        {
            printf("blob open failed\n");
        }
    }
    //  resize blob
    // 创建channel
    // 写入
    // 读取
    // close blob
    

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
    spdk_thread_exit(p_disk->m_p_thread);
    spdk_thread_destroy(p_disk->m_p_thread);

    //清理SPDK环境
    spdk_env_fini();

    free(p_disk);

    return 0;
}
