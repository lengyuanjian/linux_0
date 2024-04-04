#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <vector>
#include <set>
#include <sys/poll.h>

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
    
    pollfd poll_sockets[1024];
    int nfds = 0;
    poll_sockets[nfds].fd = server_fd;
    poll_sockets[nfds].events = POLLIN;
    nfds++;
    while(true) 
    {
        int ret = poll(poll_sockets, nfds, 500);
        if(ret == -1)
        {

        }
        else if(ret == 0)
        {}
        else
        {
            for(int i = 1; i < nfds; ++i)
            {
                if(poll_sockets[i].revents & POLLIN)
                {
                    int client = poll_sockets[i].fd;
                    char buff[4096] = {};
                    auto len = read(client, buff, 4096);
                    if(len == 0)
                    {
                        std::cout <<"df:"<< client << " disconnected\n";
                        close(client);
                        poll_sockets[i] = poll_sockets[nfds - 1];
                        nfds--;
                        i--;
                    }
                    else
                    {
                        std::cout <<"df:"<< client <<" :"<<buff<< "\n";
                        send(client,  buff, len, 0);
                    }
                }

            }
            if(poll_sockets[0].revents & POLLIN)
            {
                int new_socket;
                if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
                {
                    perror("accept error");
                }
                poll_sockets[nfds].fd = new_socket;
                poll_sockets[nfds].events = POLLIN;
                nfds++;
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &address.sin_addr, ip_str, INET_ADDRSTRLEN);

                unsigned short port = ntohs(address.sin_port);
                std::cout << "New connection, fd: " << new_socket <<"ip:"<< ip_str << ":"<< port <<"\n";
            }
        }
            
    }
    
    return 0;
}
