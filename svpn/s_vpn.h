#pragma once
#include "../lib_epoll_event/sh_socket_server.h"
#include "../vpn_define/vpv_define.h"
#include <map>
using namespace sh_net;

class vpn_server_oi;
struct vpn_channel_info
{
    vpn_server_oi * m_p_channel;
    int             m_channel_id{0};
    char            m_out_ip[32]{};
    int             m_out_port{0};
    char            m_in_ip[32]{};
    int             m_in_port{0};  
    char            m_local_ip[32]{};
    int             m_local_port{0};
};

struct vpn_client_info
{
    socket_info *   m_info;
    int             m_clinet_id{0};
    std::map<int, vpn_channel_info *> m_channel_map;
};

class vpn_server:public sh_net::socket_event
{

    sh_net::socket_server                   m_vpn_server;
    log_event *                             m_p_log{nullptr};
    protected:
    void deal_data(socket_info * info, vpn_head * p_head, char * buff);
    void wait_create_tcp_channel(vpn_creace_tcp_channel * p_creace_tcp_channel);
    void deal_client_register(socket_info * info, vpn_client_register * p_msg);
    public:
    bool send_create_channel_msg(int m_client_id, int m_channel_id);
    void send_msg_tcp_channel(int fd, const char * in_ip, int in_port, const char * local_ip, int local_port);
    protected:
    virtual void on_connected(socket_info * info);
    virtual void on_disconnected(socket_info * info);
    virtual void on_a_disconnected(socket_info * info);
    virtual int  on_recv_data(socket_info * info);
    protected:

    public:
virtual ~vpn_server(){};
    bool init(log_event * p_log, const char * ip, int port);
    bool start();
    void stop();
    void close();
public:
    bool create_channel_server(sh_fd client_id, const char * out_ip, int out_port, const char * in_ip, int in_port, const char * local_ip, int local_port);
    
    // manager clinet_id & channel_id 
    private:
    int                                            m_client_id{1};
    int                                            m_channel_id{1};
    std::map<int, sh_fd>                           m_id2vpn_map;
    std::map<sh_fd, vpn_client_info *>             m_vpn_client_map;
    public:
    void add_client(socket_info * info);
    void del_client(socket_info * info);
    vpn_channel_info * add_channel(sh_fd client_id, const char * out_ip, int out_port, const char * in_ip, int in_port, const char * local_ip, int local_port);
    void del_channel();
    void printf_client_channel();
};
