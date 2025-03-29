#pragma once
#include "../lib_epoll_event/sh_socket_server_accept.h"
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace sh_net;
class event_in;
class event_out;
class vpn_server;
class vpn_server_oi
{
    socket_server_accept                    m_out_server;
    socket_server_accept                    m_in_server;

    event_in *                              m_p_event_in;
    event_out *                             m_p_event_out;
    log_event *                             m_p_log{nullptr};
    int                                     m_client_id{0};
    int                                     m_channel_id{0};
    // 定义一个条件变量
    std::condition_variable                 m_cond;
    std::mutex                              m_mutex;
    socket_info *                           m_p_info{nullptr};

    std::condition_variable                 m_connected_cond;
    std::mutex                              m_connected_mutex;
    int                                     m_connected_count{0};

    vpn_server * m_p_vpn_server;
public:
    virtual ~vpn_server_oi(){};
    bool init(log_event * p_log, vpn_server * p_vpn_server, int client_id, int channel_id, const char * out_ip, int out_port, const char * in_ip, int in_port);
    void close();
    void notify_create_channel();
    log_event * get_log(){return m_p_log;};
public:
    void set_socket_info(socket_info * p_info){m_p_info = p_info;}
    socket_info * get_socket_info(){return m_p_info;}
    std::mutex & get_mutex(){return m_mutex;}
    std::condition_variable & get_cond(){return m_cond;}

    void add_connected(){m_connected_count++;}
    void sub_connected(){m_connected_count--;}
    bool is_connected(){return (m_connected_count>0);};
    std::mutex & get_connected_mutex(){return m_connected_mutex;}
    std::condition_variable & get_connected_cond(){return m_connected_cond;}
};
