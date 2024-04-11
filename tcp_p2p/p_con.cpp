// g++ -std=c++20 -o app_con connect.cpp 
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include <thread>
#include <sys/epoll.h>
#include <atomic>

char ip_str[INET_ADDRSTRLEN];

int main(int argc, char * argv[]) 
{
    if(argc < 5){
        std::cerr << "input argc < 5" << std::endl;
        return 0;
    }
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Error: Unable to create socket" << std::endl;
        return 1;
    }
    // 设置本地地址    
    struct sockaddr_in local_addr;
    std::memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(argv[1]); // 本地IP地址
    local_addr.sin_port = htons(std::atoi(argv[2])); // 本地端口号

    // 绑定到本地地址
    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&local_addr), sizeof(local_addr)) == -1) {
        std::cerr << "Error: Unable to bind to local address" << std::endl;
        close(sockfd);
        return 1;
    }
    
    inet_ntop(AF_INET, &(local_addr.sin_addr), ip_str, INET_ADDRSTRLEN); 
    std::cout << "Local IP: " << ip_str << " : " << ntohs(local_addr.sin_port) << std::endl;

    // 设置服务器地址
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[3]);  
    server_addr.sin_port = htons(std::atoi(argv[4]));  
 
    // 连接到服务器
    while (connect(sockfd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        static unsigned long long count = 0;
        std::cerr << "\rError: Unable to connect to server："<<count++;
        fflush(stderr);
        // close(sockfd);
        // return 1;
    }
     
    send(sockfd,ip_str, strlen(ip_str) ,0);
    std::cout  << std::endl << "Connected to server successfully!" << std::endl;
 
    inet_ntop(AF_INET, &(server_addr.sin_addr), ip_str, INET_ADDRSTRLEN); 
    std::cout << "Remote IP: " << ip_str << " : " << ntohs(server_addr.sin_port) << std::endl;
    std::atomic<bool> b_running(true); // 使用原子变量来控制线程是否运行

    // 接收数据的线程
    std::thread rx_thread([&sockfd, &b_running]() {
        int epfd = epoll_create(1);
        if (epfd == -1) 
        {
            std::cerr << "Error: Unable to create epoll instance" << std::endl;
            b_running = false;
            return;
        }

        // 添加 sockfd 到 epoll 实例中
        struct epoll_event ev;
        ev.events = EPOLLIN; // 设置监听事件为可读和边缘触发模式
        ev.data.fd = sockfd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) 
        {
            std::cerr << "Error: Unable to add sockfd to epoll" << std::endl;
            b_running = false;
            return;
        }
        while(b_running)
        {
            struct epoll_event events;
            int num_events = epoll_wait(epfd, &events, 1, 100);
            if (num_events == -1) 
            {
                std::cerr << "Error: epoll_wait() failed" << std::endl;
                b_running = false;
                break;
            }
            else if(num_events == 0)
            {

            }else
            {
                int fd = events.data.fd;
                char buff[4096] = {};
                int len = recv(fd, buff, sizeof(buff), 0);
                if (len > 0) 
                {
                    buff[len] = '\0';
                    std::cout << "Received: " << buff << std::endl;
                } 
                else if (len == 0) 
                {
                    std::cerr << "Error: Connection closed by remote host" << std::endl;
                    b_running = false;
                    break;
                } 
                else 
                {
                    std::cerr << "Error: recv() failed" << std::endl;
                    b_running = false;
                    break;
                }
            } 
        }
        close(epfd);
    }); 
    while(b_running)
    {
        char imput_str[1024] = {};
        if(fgets(imput_str, 1024, stdin))
        {
            int len = strlen(imput_str);
            if(len > 1)
            {
                if(imput_str[0] =='q')
                {
                    b_running = false;
                    break;
                }
                imput_str[len - 1] = 0;
                send(sockfd, imput_str, len - 1, 0);
            }
        }
    }

    rx_thread.join();
    std::cout<<"exit...\n";
    getchar(); 
    close(sockfd);
    

    return 0;
}
