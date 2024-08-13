#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef int (*sh_open)(const char *pathname, int flags, ...);

#include <unistd.h>

typedef ssize_t (*sh_read)(int fd, void *buf, size_t count);
typedef ssize_t (*sh_write)(int fd, const void *buf, size_t count);
typedef int (*sh_close)(int fd);
typedef off_t (* sh_lseek)(int fd, off_t offset, int whence);

extern "C"
{

int open(const char *pathname, int flags, ...)
{
    if(flags & O_CREAT)
    {
        va_list args;
    }
    else
    {

    }
    return 
}

ssize_t read(int fd, void *buf, size_t count)
{
    return sh_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    return sh_write(fd, buf, count);
}

int close(int fd)
{
    return sh_close(fd);
}

//#include <sys/types.h>
//#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence)
{
    return sh_lseek(fd, offset, whence);
}

}