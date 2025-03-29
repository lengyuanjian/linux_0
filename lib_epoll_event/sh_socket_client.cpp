#include "sh_socket_client.h"
#include "sh_socket_define.h"
#include <sys/socket.h> //getsockname
#include <cstdio> 
#include <arpa/inet.h> // inet_pton
#include <unistd.h>
#include <string.h>

namespace sh_net 
{
    bool socket_client::init(socket_event * p_event, log_event * p_log)
    {
        m_p_event = p_event;
        m_p_log = p_log;
        m_debug_out = m_info_out = stdout;
        m_error_out = stderr;

        m_epfd = epoll_create(1);
        return m_epfd > 0;
    }
    bool socket_client::start()
    {
        m_epoll_thread = std::thread(&socket_client::run_epoll, this);
        return true;
    }
    void socket_client::stop()
    {
        m_epoll_run = false;
        m_epoll_thread.join();
    }
    void socket_client::close()
    {
        m_socket_manager.close();
    }
    void socket_client::close(socket_info * p_info)
    {
        epoll_event ev;
        ev.events = EPOLLOUT;
        ev.data.ptr = p_info;
        epoll_ctl(m_epfd, EPOLL_CTL_MOD, p_info->m_fd, &ev);
    }
    socket_info * socket_client::connect(const char *ip, unsigned short port, const char *local_ip, unsigned short local_port)
    {
        int client_fd;
        if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        {
            m_p_log->on_error(m_error_out,"socket failed");
            return nullptr;
        } 

        if (local_ip != nullptr)
        {
            sockaddr_in local_address;
            local_address.sin_family = AF_INET;
            inet_pton(AF_INET, local_ip, &local_address.sin_addr);
            local_address.sin_port = htons(local_port);
            if (bind(client_fd, (struct sockaddr *)&local_address, sizeof(local_address)) < 0)
            {
                int port = ntohs(local_address.sin_port); 
                char str_ip[32]={};
                inet_ntop(AF_INET, &local_address.sin_addr, str_ip, sizeof(str_ip));
                m_p_log->on_error(m_error_out,"bind failed[%s:%d]",ip,port);
                return nullptr;
            }
            else 
            {
                sockaddr_in local_address;
                socklen_t local_address_len = sizeof(local_address);
                getsockname(client_fd, (struct sockaddr *)&local_address, &local_address_len);
                int port = ntohs(local_address.sin_port); 
                char str_ip[32]={};
                inet_ntop(AF_INET, &local_address.sin_addr, str_ip, sizeof(str_ip));
                m_p_log->on_debug(m_debug_out,"client bind[%s:%d]",str_ip,port);
                    
            }
        }

        sockaddr_in address;
        int addrlen = sizeof(address);
        address.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &address.sin_addr);
        address.sin_port = htons(port);
        int  d_port = ntohs(address.sin_port); 
        char str_ip[32]={};
        inet_ntop(AF_INET, &address.sin_addr, str_ip, sizeof(str_ip));
        if (::connect(client_fd, (struct sockaddr *)&address, addrlen) < 0) 
        {
            m_p_log->on_error(m_error_out,"connect failed[%s:%d]",str_ip,d_port);
            return nullptr;
        }
        socket_info * info = m_socket_manager.get_clinet_info(client_fd);
        m_p_log->on_debug(m_debug_out,"connect success" sh_net_socket_format , sh_net_socket_args(info));
        
        return info;
    }
    void socket_client::add_eool(socket_info * info)
    {
        m_p_log->on_info(m_info_out, "connected " sh_net_socket_format , sh_net_socket_args(info));
        m_p_event->on_connected(info);
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.ptr = info;
        info->m_epfd = m_epfd;
        epoll_ctl(m_epfd, EPOLL_CTL_ADD,info->m_fd,&ev);
    }
    void socket_client::run_epoll()
    {
        epoll_event evs[4096] = {};
        while(m_epoll_run)
        {
            int nready = epoll_wait(m_epfd, evs, 4096, 1000);
            if(nready < 0)
            {
                m_p_log->on_error(m_error_out,"epoll_wait failed");
                break;
            }
            for(int i = 0; i < nready; ++i)
            {
                socket_info * info = (socket_info *)(evs[i].data.ptr);
                int sock = info->m_fd;
                if(evs[i].events & EPOLLOUT)
                {
                    m_p_log->on_info(m_info_out, "a disconnected " sh_net_socket_format , sh_net_socket_args(info));
                    m_p_event->on_a_disconnected(info);
                    ::close(sock);
                    epoll_ctl(m_epfd, EPOLL_CTL_DEL,sock,nullptr);
                    m_socket_manager.del_clinet_info(info);
                }
                else if(evs[i].events & EPOLLIN)
                {
                    auto p_data = &(info->m_data);
                    auto len = recv(sock, p_data->m_buff + p_data->m_buff_len, p_data->m_capacity - p_data->m_buff_len, 0);
                    if(len == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            m_p_log->on_info(m_info_out,"recv EAGAIN  " sh_net_socket_format , sh_net_socket_args(info));
                        }
                        else 
                        {
                            m_p_log->on_info(m_info_out, "a_disconnected  " sh_net_socket_format , sh_net_socket_args(info));
                            m_p_event->on_a_disconnected(info);
                            ::close(sock);
                            epoll_ctl(m_epfd, EPOLL_CTL_DEL,sock,nullptr);
                            m_socket_manager.del_clinet_info(info);
                        }
                        continue;
                    }
                    info->m_data.m_buff_len += len;
                    if(len == 0)
                    {
                        m_p_log->on_info(m_info_out, "disconnected " sh_net_socket_format , sh_net_socket_args(info));
                        m_p_event->on_disconnected(info);
                        ::close(sock);
                        epoll_ctl(m_epfd, EPOLL_CTL_DEL,sock,nullptr);
                        m_socket_manager.del_clinet_info(info);
                    }
                    else
                    {
                        int use_len = m_p_event->on_recv_data(info);
                        if(use_len < info->m_data.m_buff_len)
                        {
                            if(use_len > 0)
                            {
                                memmove(info->m_data.m_buff, info->m_data.m_buff + use_len, info->m_data.m_buff_len - use_len);
                                info->m_data.m_buff_len -= use_len;
                            }
                        }
                        else
                        {
                            info->m_data.m_buff_len = 0;
                        }
                    }
                }
            }
        }
        ::close(m_epfd);
    }  
}
