#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main() {
    // 要分配的内存大小（例如：2MB）
    size_t length = 2 * 1024 * 1024;

    // 打开hugepage文件
    int fd = open("/mnt/huge/hugepagefile", O_CREAT | O_RDWR, 0755);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 为文件分配空间
    if (ftruncate(fd, length) == -1) {
        perror("ftruncate");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 使用mmap分配大页内存
    void *addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Hugepage memory allocated at address %p\n", addr);

    // 使用内存
    // 例如：写入数据
    int *data = (int *)addr;
    for (size_t i = 0; i < length / sizeof(int); i++) {
        data[i] = i;
    }

    // 解除映射和关闭文件
    if (munmap(addr, length) == -1) {
        perror("munmap");
    }
    close(fd);

    return 0;
}
