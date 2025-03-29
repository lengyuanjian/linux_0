#include "sh_socket_server_accept.h"
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h> // inet_pton
#include <unistd.h>
#include <string.h>

namespace sh_net 
{
    bool socket_server_accept::init(socket_event * p_event, log_event * p_log, const char *ip, int port)
    {
        m_p_event = p_event;
        m_p_log = p_log;
        m_debug_out = m_info_out = stdout;
        m_error_out = stderr;
        m_ip = ip;
        m_port = port;
        if((m_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        {
            m_p_log->on_error(m_error_out,"socket create failed");
            return false;
        }
        int opt = 1;
        if(setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            m_p_log->on_error(m_error_out,"socket setsockopt failed!");
            ::close(m_server_fd);
            return false;
        }
        struct sockaddr_in address;
        address.sin_family = AF_INET;   
        inet_pton(AF_INET, ip, &address.sin_addr);
        address.sin_port = htons((unsigned short)port);
        if (bind(m_server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
        {
            m_p_log->on_error(m_error_out,"socket bind failed![%s:%d]",ip,port);
            return false;
        }
        m_p_log->on_debug(m_debug_out,"server bind[%s:%d]",ip,port);
        return true; 
    }
    bool socket_server_accept::start()
    {
        if (listen(m_server_fd, 10) < 0) 
        {
            m_p_log->on_error(m_error_out,"listen failed[%s:%d]",m_ip.c_str(),m_port);
            return false;
        }
        m_p_log->on_info(m_info_out,"server listen[%s:%d]",m_ip.c_str(),m_port);
        m_epoll_thread = std::thread(&socket_server_accept::run_epoll, this);
        m_accept_thread = std::thread(&socket_server_accept::run_accept, this);
        return true;
    }
    void socket_server_accept::stop()
    {
        m_p_log->on_info(m_info_out,"server accept stop[%s:%d]",m_ip.c_str(),m_port);
        m_epoll_run = false;
        m_accept_run = false;
        m_epoll_thread.join();
        m_accept_thread.join();
    }
    void socket_server_accept::close()
    {
        m_p_log->on_info(m_info_out,"server accept close[%s:%d]",m_ip.c_str(),m_port);
        for(auto & [_, info] : m_socket_manager)
        {
            m_p_log->on_info(m_info_out, "close disconnected " sh_net_socket_format , sh_net_socket_args(info));
            m_p_event->on_disconnected(info);
        }
        m_socket_manager.close();
        ::close(m_server_fd);
    }
    void socket_server_accept::close(socket_info * p_info)
    {
        epoll_event ev;
        ev.events = EPOLLOUT;
        ev.data.ptr = p_info;
        epoll_ctl(m_epfd, EPOLL_CTL_MOD, p_info->m_fd, &ev);
    }
    void socket_server_accept::add_eool(socket_info * info)
    {
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.ptr = info;
        info->m_epfd = m_epfd;
        epoll_ctl(m_epfd, EPOLL_CTL_ADD,info->m_fd,&ev);
        
        m_p_log->on_info(m_info_out, "connected " sh_net_socket_format , sh_net_socket_args(info));
        m_p_event->on_connected(info);
    }
    void socket_server_accept::run_epoll()
    {
        m_epfd = epoll_create(1); // 参数只要大于0就可以
        epoll_event evs[4096] = {};
        while(m_epoll_run)
        {
            int nready = epoll_wait(m_epfd, evs, 4096, 1000);
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
                            m_p_log->on_info(m_info_out,"recv EAGAIN " sh_net_socket_format "[%d]",sh_net_socket_args(info),errno);
                        }
                        else 
                        {
                            m_p_log->on_info(m_info_out, "a_disconnected " sh_net_socket_format "[%d]",sh_net_socket_args(info),errno);
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
        m_p_log->on_info(m_info_out,"epoll thread exit");
        ::close(m_epfd);
    }
    void socket_server_accept::run_accept()
    {
        m_accept_epfd = epoll_create(1); // 参数只要大于0就可以
        epoll_event ev;
        ev.events = EPOLLIN;
        epoll_ctl(m_accept_epfd, EPOLL_CTL_ADD, m_server_fd, &ev);
        epoll_event evs[1] = {};
        while(m_accept_run)
        {
            int nready = epoll_wait(m_accept_epfd, evs, 1, 1000);
            for(int i = 0; i < nready; ++i)
            {
                if(evs[i].events & EPOLLIN)
                {
                    int new_socket;
                    sockaddr_in address;
                    int addrlen = sizeof(sockaddr_in);
                    if ((new_socket = accept(m_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
                    {
                        m_p_log->on_error(m_error_out,"accept error! [%s:%d]",m_ip.c_str(),m_port);
                    }
                    else
                    {
                        socket_info * info = m_socket_manager.get_clinet_info(new_socket);
                        m_p_log->on_info(m_info_out, "accept " sh_net_socket_format , sh_net_socket_args(info));
                        m_p_event->on_accept(info);
                        add_eool(info);
                    }
                }
            }
        }
        m_p_log->on_info(m_info_out,"accept epoll thread exit");
        ::close(m_accept_epfd);
    }
}
