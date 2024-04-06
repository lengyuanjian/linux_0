#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <vector>
#include <set>
#include <sys/poll.h>
#include <sys/epoll.h>

int main(int argc, char * argv[]) 
{
    if(argc < 3)
    {
        std::cout<<"./exec ip port\n";
        return 1;
    }

    const char * ip = argv[1];
    unsigned short port = std::atoi(argv[2]); 

    int server_fd;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("socket failed");
        return 1;
    } 
    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        std::cout<<"bind "<< ip << ":"<< port <<"\n";
        perror("bind failed");
        return 1;
    }
    
    // 监听连接请求
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        return 1;
    }
    std::cout<<"fd:" << server_fd <<"server "<< ip << ":"<< port <<"\n";
    
    int epfd = epoll_create(1); // 参数只要大于0就可以
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    epoll_ctl(epfd, EPOLL_CTL_ADD,server_fd,&ev);
    epoll_event evs[1024] = {};
    while(1)
    {
        int nready = epoll_wait(epfd, evs,1024,-1);
        for(int i = 0; i < nready; ++i)
        {
            int sock = evs[i].data.fd;
            if(sock == server_fd)
            {
                int new_socket;
                if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
                {
                    perror("accept error");
                }
                ev.events = EPOLLIN;
                ev.data.fd = new_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD,new_socket,&ev);
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &address.sin_addr, ip_str, INET_ADDRSTRLEN);

                unsigned short port = ntohs(address.sin_port);
                std::cout << "New connection, fd: " << new_socket <<"ip:"<< ip_str << ":"<< port <<"\n";
            }
            else if(evs[i].events & EPOLLIN)
            {
                char buff[4096] = {};
                auto len = read(sock, buff, 4096);
                if(len == 0)
                {
                    std::cout <<"df:"<< sock << " disconnected\n";
                    close(sock);
                    epoll_ctl(epfd, EPOLL_CTL_DEL,sock,nullptr);
                }
                else
                {
                    std::cout <<"df:"<< sock <<" :"<<buff<< "\n";
                    send(sock,  buff, len, 0);
                }
            }
        }
    }
    
    return 0;
}
