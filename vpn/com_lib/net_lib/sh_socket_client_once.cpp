#include "sh_socket_client_once.h"
#include "sh_socket_define.h"
#include <sys/socket.h> //getsockname
#include <cstdio> 
#include <arpa/inet.h> // inet_pton
#include <unistd.h>
#include <string.h>
namespace sh_net 
{
    bool socket_client_once::init(socket_event_once * p_event, log_event * p_log,
        const char *ip, unsigned short port, const char *local_ip, unsigned short local_port, int auto_reconnect_s)
    {
        m_p_event = p_event;
        m_p_log = p_log;
        m_auto_reconnect_s = auto_reconnect_s;
        m_debug_out = m_info_out = stdout;
        m_error_out = stderr;
        
        m_ip = ip;
        m_port = port;
        if(local_ip)
        {
            m_local_ip = local_ip;
        }
        m_local_port = local_port;
        m_epfd = epoll_create(1);
        return m_epfd > 0;
    }
    bool socket_client_once::start()
    {
        m_epoll_run = true;
        m_epoll_thread = std::thread(&socket_client_once::run_epoll, this);
        
        return true;
    }
    void socket_client_once::stop()
    {
        m_epoll_run = false;
        m_auto_reconnect_s = 0;
        m_epoll_thread.join();
    }
    void socket_client_once::close()
    {
    }
    bool socket_client_once::connect(const char *ip, unsigned short port, const char *local_ip, unsigned short local_port)
    {
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) 
        {
            m_p_log->on_error(m_error_out, "socket creation failed");
            return false;
        }

        if (local_ip != nullptr)
        {
            sockaddr_in local_address = {};
            local_address.sin_family = AF_INET;
            local_address.sin_port = htons(local_port);
            if (inet_pton(AF_INET, local_ip, &local_address.sin_addr) <= 0)
            {
                m_p_log->on_error(m_error_out, "invalid local IP address");
                ::close(client_fd);
                return false;
            }

            if (bind(client_fd, (struct sockaddr *)&local_address, sizeof(local_address)) < 0)
            {
                m_p_log->on_error(m_error_out, "bind failed [%s:%d]", local_ip, local_port);
                ::close(client_fd);
                return false;
            }

            sockaddr_in bound_address = {};
            socklen_t bound_address_len = sizeof(bound_address);
            if (getsockname(client_fd, (struct sockaddr *)&bound_address, &bound_address_len) == 0)
            {
                m_socket_info.m_local_port = ntohs(bound_address.sin_port);
                inet_ntop(AF_INET, &bound_address.sin_addr, m_socket_info.m_local_ip, sizeof(m_socket_info.m_local_ip));
                m_p_log->on_debug(m_debug_out, "client bound to [%s:%d]", m_socket_info.m_local_ip, m_socket_info.m_local_port);
            }
        }

        sockaddr_in remote_address = {};
        remote_address.sin_family = AF_INET;
        remote_address.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &remote_address.sin_addr) <= 0)
        {
            m_p_log->on_error(m_error_out, "invalid remote IP address");
            ::close(client_fd);
            return false;
        }

        if (::connect(client_fd, (struct sockaddr *)&remote_address, sizeof(remote_address)) < 0) 
        {
            m_p_log->on_error(m_error_out, "connect failed [%s:%d]", ip, port);
            ::close(client_fd);
            return false;
        }

        sockaddr_in connected_address = {};
        socklen_t connected_address_len = sizeof(connected_address);
        if (getsockname(client_fd, (struct sockaddr *)&connected_address, &connected_address_len) == 0)
        {
            m_socket_info.m_local_port = ntohs(connected_address.sin_port);
            inet_ntop(AF_INET, &connected_address.sin_addr, m_socket_info.m_local_ip, sizeof(m_socket_info.m_local_ip));
        }

        m_socket_info.m_remve_port = port;
        strncpy(m_socket_info.m_remve_ip, ip, sizeof(m_socket_info.m_remve_ip) - 1);

        m_p_log->on_debug(m_debug_out, "connect success " sh_net_socket_format, sh_net_socket_args(&m_socket_info));
        m_socket_info.m_fd = client_fd;
        m_socket_info.m_epfd = m_epfd;

        m_p_log->on_info(m_info_out, "connected " sh_net_socket_format, sh_net_socket_args(&m_socket_info));
        m_p_event->on_connected(&m_socket_info);

        epoll_event ev = {};
        ev.events = EPOLLIN;
        ev.data.ptr = &m_socket_info;
        if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, client_fd, &ev) < 0)
        {
            m_p_log->on_error(m_error_out, "epoll_ctl failed");
            ::close(client_fd);
            return false;
        }

        return true;
    }
    void socket_client_once::run_epoll()
    {
        auto loop_connect = [this](int interval_seconds)
        {
            const char *local_ip = m_local_ip.empty() ? nullptr : m_local_ip.c_str();
            while (m_epoll_run)
            {
                if (connect(m_ip.c_str(), m_port, local_ip, m_local_port))
                {
                    break; // Successfully connected
                }

                for (int elapsed = 0; elapsed < interval_seconds && m_epoll_run; ++elapsed)
                {
                    sleep(1); // Wait for 1 second before retrying
                }
            }
        };
        loop_connect(5);
        epoll_event evs[2] = {};
        while(m_epoll_run)
        {
            int nready = epoll_wait(m_epfd, evs, 2, 1000);
            if(nready < 0)
            {
                m_p_log->on_error(m_error_out,"epoll_wait failed");
                break;
            }
            if(nready == 0)
            {
                m_p_event->on_time_out(&m_socket_info);
                continue;
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
                    
                    if(m_auto_reconnect_s > 0)
                    {
                        loop_connect(m_auto_reconnect_s);
                    }
                    else
                    {
                        m_epoll_run = false;
                        break;
                    }
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
                            if(m_auto_reconnect_s > 0)
                            {
                                loop_connect(m_auto_reconnect_s);
                            }
                            else
                            {
                                m_epoll_run = false;
                                break;
                            }
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
                        if(m_auto_reconnect_s > 0)
                        {
                            loop_connect(m_auto_reconnect_s);
                        }
                        else
                        {
                            m_epoll_run = false;
                            break;
                        }
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
