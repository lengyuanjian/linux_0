#pragma once
#include "interface/vpn_context.h"
#include "../com_lib/net_lib/sh_socket_server.h"
#include "../com_lib/protocol/vpv_protocol.h"

struct vpn_client_info
{
    int                     m_clinet_id{0};
    sh_net::socket_info *   m_info{nullptr};
    std::map<int, vpn_channel_info *> m_channel_map;
};

class business_server : public i_business_server , public sh_net::socket_event
{
    sh_net::socket_server                   m_vpn_server;
    vpn_context *                           m_p_vpn_context{nullptr};
public:
    ~business_server(){}
    bool init(vpn_context * p_vpn_context, const char * ip, int port);
    bool start();
    void stop();
    void close();
public:
    void send_msg_tcp_channel(int fd, const char * server_ip, int server_port, const char * local_ip, int local_port);
    bool send_create_channel_msg(int m_client_id, int m_channel_id);
    vpn_channel_info * create_channel_server(int client_id, const char * out_ip, const char * out_ip1, int out_port, const char * in_ip, const char * in_ip1, int in_port, const char * local_ip, int local_port);
protected:
    virtual void on_connected(sh_net::socket_info * info);
    virtual void on_disconnected(sh_net::socket_info * info);
    virtual void on_a_disconnected(sh_net::socket_info * info);
    virtual int  on_recv_data(sh_net::socket_info * info);
    void deal_data(sh_net::socket_info * info, vpn_head * p_head, char * buff);
    void deal_client_register(sh_net::socket_info * info, vpn_client_register * p_msg);
private:
    std::map<int, vpn_client_info *>        m_clinet_map;
protected:
};
