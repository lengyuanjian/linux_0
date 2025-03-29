#pragma once
// #include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

namespace sh_net 
{
    using sh_fd=unsigned int;
    struct socket_data
    {
        int  m_capacity{8*1024};
        int  m_buff_len{0};
        void *m_ptr{nullptr};
        char m_buff[8*1024];
    };
    struct socket_info
    {
        int          m_fd{0};
        int          m_epfd{0};
        sh_fd        m_sh_fd{0};
        char         m_local_ip[32]{};
        int          m_local_port{0};
        char         m_remve_ip[32]{};
        int          m_remve_port{0};
        socket_data  m_data;
    };
    
#define sh_net_socket_format "[%u][%s:%d]->[%s:%d]" 
#define sh_net_socket_args(info) (info)->m_sh_fd,(info)->m_local_ip,(info)->m_local_port, (info)->m_remve_ip,(info)->m_remve_port

    inline void get_local_socket_addr(int fd,const char *ip_str, int *port)
    {
        sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getsockname(fd, (struct sockaddr*)&addr, &addr_len) == -1) 
        {
            return;
        }
        inet_ntop(AF_INET, &addr.sin_addr, (char*)ip_str, INET_ADDRSTRLEN);
        *port = ntohs(addr.sin_port);
    }
    inline void get_remote_socket_addr(int fd, const char * ip_str, int *port)
    {
        sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getpeername(fd, (struct sockaddr*)&addr, &addr_len) == -1) 
        {
            return;
        }
        inet_ntop(AF_INET, &addr.sin_addr, (char*)ip_str, INET_ADDRSTRLEN);
        *port = ntohs(addr.sin_port);
    }
    inline void close_socket(socket_info * p_info)
    {
        epoll_event ev;
        ev.events = EPOLLOUT;
        ev.data.ptr = p_info;
        epoll_ctl(p_info->m_epfd, EPOLL_CTL_MOD, p_info->m_fd, &ev);
    }
    inline void set_socket_addr(socket_info * p_info)
    {
        get_local_socket_addr(p_info->m_fd, p_info->m_local_ip, &p_info->m_local_port);
        get_remote_socket_addr(p_info->m_fd, p_info->m_remve_ip, &p_info->m_remve_port);
    }
    class socket_event
    {
    public:
        virtual ~socket_event(){}
        virtual void on_accept(socket_info * info){(void)info;};
        virtual void on_connected(socket_info * info) = 0;
        virtual void on_disconnected(socket_info * info) = 0;
        virtual void on_a_disconnected(socket_info * info) = 0;
        virtual int  on_recv_data(socket_info * info) = 0;
    };
}
