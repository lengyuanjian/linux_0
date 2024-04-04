#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <vector>
#include <set>

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
    
    std::set<int> client_sockets;

    while(true) 
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        auto max_sd = server_fd;

        for(auto const& sock : client_sockets) 
        {
            FD_SET(sock, &readfds);
            max_sd = std::max(max_sd, sock);
        }
        auto activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);

        if ((activity < 0) && (errno != EINTR)) 
        {
            perror("select error");
        }
        if(FD_ISSET(server_fd, &readfds))
        {
            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
            {
                perror("accept error");
            }
            client_sockets.insert(new_socket);
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &address.sin_addr, ip_str, INET_ADDRSTRLEN);

            unsigned short port = ntohs(address.sin_port);
            std::cout << "New connection, fd: " << new_socket <<"ip:"<< ip_str << ":"<< port <<"\n";
        }
        std::set<int> del_client;
        for(auto & client : client_sockets)
        {
            char buff[4096] = {0};
            if(FD_ISSET(client, &readfds))
            {
                auto len = read(client, buff, 4096);
                if(len == 0)
                {
                    std::cout <<"df:"<< client << " disconnected\n";
                    close(client);
                    del_client.insert(client);
                }
                else
                {
                    std::cout <<"df:"<< client <<" :"<<buff<< "\n";
                    send(client,  buff, len, 0);

                }
            }
        }
        for(auto & client : del_client)
        {
            client_sockets.erase(client);
        }
        
    }
    
    return 0;
}
