#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    const char *pathname = "example.txt";
    
    // 打开或创建文件
    int fd = open(pathname, O_RDWR | O_CREAT, 0644);
    if (fd < 0) 
    {
        perror("open failed");
        return 1;
    }
    // 读取文件内容
    char buffer[50] = {};
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) 
    {
        perror("read failed");
        close(fd);
        return 1;
    }
    else if(bytes_read == 0)
    {
        printf("file empty: %s\n", buffer);
        const char *text = "1:Hello, World!";
        if (write(fd, text, strlen(text)) < 0) 
        {
            perror("write failed");
            close(fd);
            return 1;
        }
        printf("file write: %s\n", text);
        close(fd);
        return 1;
    }
    buffer[bytes_read] = '\0';  // 添加字符串终止符
    printf("Read from file: %s\n", buffer);

    lseek(fd, 0, SEEK_SET);
    buffer[0] = (buffer[0] >= '9' || buffer[0] < '0') ? '0' : buffer[0] + 1;
    if (write(fd, buffer, bytes_read) < 0) 
    {
        perror("write failed");
        close(fd);
        return 1;
    }
    
    
    // 关闭文件
    if (close(fd) < 0) {
        perror("close failed");
        return 1;
    }

    return 0;
}
