#include <iostream>
#include "../lib_epoll_event/sh_socket_client.h"




int main1(int argc, char * argv[]) 
{
    if(argc < 3)
    {
        std::cout<<"./exec ip port\n";
        return 1;
    }

    const char * ip = argv[1];
    unsigned short port = std::atoi(argv[2]); 
    
    return 0;
}
