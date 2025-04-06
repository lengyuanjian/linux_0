#pragma once
#include "sh_log.h"
#include "sh_socket_define.h"
#include "sh_socket_manager.h"
#include <map>
#include <thread>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace sh_net 
{
    class socket_client    
    {
        socket_manager          m_socket_manager;
        socket_event *          m_p_event;
        log_event *             m_p_log;
        FILE *                  m_debug_out;
        FILE *                  m_info_out;
        FILE *                  m_error_out;
    private:
        int                     m_server_fd{0};
        std::string             m_ip;
        int                     m_port{0};
        int                     m_epfd{0};
        std::thread             m_epoll_thread;
        bool                    m_epoll_run{true};
    public:
        bool init(socket_event * p_event, log_event * p_log);
        bool start();
        void stop();
        void close();
        void close(socket_info * p_info);
        socket_info * connect(const char *ip, unsigned short port, const char *local_ip = nullptr, unsigned short local_port = 0);
        void add_eool(socket_info * clinet_info);
        socket_info * get_clinet_info(int fd);
    protected:
        void run_epoll();
    };
}
