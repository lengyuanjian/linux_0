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
    class socket_server_accept    
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
        int                     m_accept_epfd{0};
        int                     m_epfd{0};
        std::thread             m_epoll_thread;
        std::thread             m_accept_thread;
        bool                    m_epoll_run{true};
        bool                    m_accept_run{true};
    public:
        bool init(socket_event * p_event, log_event * p_log, const char *ip, int port);
        bool start();
        void stop();
        void close();
        void close(socket_info * p_info);
        int  get_server_fd() { return m_server_fd; }
    protected:
        void add_eool(socket_info * clinet_info);
        void run_epoll();
        void run_accept();
    };
}
