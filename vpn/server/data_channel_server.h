#pragma once
#include "interface/vpn_context.h"
#include "../com_lib/net_lib/sh_socket_client.h"
#include <map>
#include <queue>

struct server_socket_info
{
    int             m_fd{0};
    int             m_epfd{0};
    int             m_type{0};
    int             m_client_id;
    int             m_channel_id;
    char            m_local_ip[32]{};
    int             m_local_port{0};
    std::queue<sh_net::socket_info *> m_free_socket;
    std::queue<sh_net::socket_info *> m_busy_socket;
    server_socket_info * m_p_removte{nullptr};
};


class data_channel_server: public i_data_channel_server, public sh_net::socket_event
{
    vpn_context *                           m_p_vpn_context;
    std::thread                             m_accept_thread;
    bool                                    m_accept_run{true};
    int                                     m_accept_epfd{0};
public:
    virtual ~data_channel_server(){}
    bool init(vpn_context * p_vpn_context);
    bool start();
    void stop();
    void close();
protected:
    virtual void on_connected(sh_net::socket_info * info);
    virtual void on_disconnected(sh_net::socket_info * info);
    virtual void on_a_disconnected(sh_net::socket_info * info);
    virtual int  on_recv_data(sh_net::socket_info * info);
protected:  
    void run_accept();
    int server_sock_init(const char *ip, int port);
    server_socket_info * init_server_socket(const char * out_ip, int out_port,const char * in_ip, int in_port);
    bool server_add_eool(server_socket_info * server_info);
    void server_delete_eool(server_socket_info * server_info);
    void date_socket_accept(server_socket_info * server_info, sh_net::socket_info * info);

public:
    bool create_channel_server(vpn_channel_info * p_channel_info);
    bool delete_channel_server(int client_id, int channel_id);
private:
    sh_net::socket_client                   m_vpn_data_socket;
    std::map<long long, server_socket_info *> m_channel_map;
    
};
