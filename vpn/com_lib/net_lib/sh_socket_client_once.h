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
    class socket_event_once 
    {
    public:
        virtual ~socket_event_once(){}
        virtual void on_time_out(socket_info * info){(void)info;};
        virtual void on_connected(socket_info * info) = 0;
        virtual void on_disconnected(socket_info * info) = 0;
        virtual void on_a_disconnected(socket_info * info) = 0;
        virtual int  on_recv_data(socket_info * info) = 0;
    };
    
    class socket_client_once    
    {
        socket_event_once *     m_p_event;
        log_event *             m_p_log;
        int                     m_auto_reconnect_s{0};       
        FILE *                  m_debug_out;
        FILE *                  m_info_out;
        FILE *                  m_error_out;
    private:
        int                     m_epfd{0};
        socket_info             m_socket_info;
        std::thread             m_epoll_thread;
        bool                    m_epoll_run{false};
        std::string             m_ip;
        int                     m_port{0};
        std::string             m_local_ip;
        int                     m_local_port{0};
    public:
        bool init(socket_event_once * p_event, log_event * p_log, const char *ip, unsigned short port, const char *local_ip, unsigned short local_port,int auto_reconnect_s = 0);
        bool start();
        void stop();
        void close();
        bool connect(const char *ip, unsigned short port, const char *local_ip = nullptr, unsigned short local_port = 0);
    protected:
        void run_epoll();
    };
}
