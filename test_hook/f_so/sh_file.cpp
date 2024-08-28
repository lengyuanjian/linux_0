
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

typedef int (*sh_open_t)(const char *pathname, int flags, ...);
typedef int (*sh_close_t)(int fd);
typedef ssize_t (*sh_read_t)(int fd, void *buf, size_t count);
typedef ssize_t (*sh_write_t)(int fd, const void *buf, size_t count);
typedef off_t (* sh_lseek_t)(int fd, off_t offset, int whence);

sh_open_t sh_open = NULL;
sh_close_t sh_close = NULL;
sh_read_t sh_read = NULL;
sh_write_t sh_write = NULL;
sh_lseek_t sh_lseek = NULL;

__attribute__((constructor)) void sh_init() 
{
    sh_open = (sh_open_t)dlsym(RTLD_NEXT, "open");
    sh_close = (sh_close_t)dlsym(RTLD_NEXT, "close");
    sh_read = (sh_read_t)dlsym(RTLD_NEXT, "read");
    sh_write = (sh_write_t)dlsym(RTLD_NEXT, "write");
    sh_lseek = (sh_lseek_t)dlsym(RTLD_NEXT, "lseek");
}

extern "C"
{

// void *dlsym(void *handle, const char *symbol);

int open(const char *pathname, int flags, ...)
{
    printf("sh_%s\n",__func__);
    if(flags & O_CREAT)
    {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return sh_open(pathname, flags, mode);
    }

    return sh_open(pathname, flags); 
}

ssize_t read(int fd, void *buf, size_t count)
{
    printf("sh_%s\n",__func__);
    return sh_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    printf("sh_%s\n",__func__);
    return sh_write(fd, buf, count);
}

int close(int fd)
{
    printf("sh_%s\n",__func__);
    return sh_close(fd);
}

off_t lseek(int fd, off_t offset, int whence)
{
    printf("sh_%s\n",__func__);
    return sh_lseek(fd, offset, whence);
}

}