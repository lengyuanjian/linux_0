#pragma once
#include "vpn_define.h"

class vpn_context;
class i_business_server
{
public:
    virtual ~i_business_server(){}
    virtual bool init(vpn_context * p_vpn_context, const char * ip, int port) = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void close() = 0;
public:
    virtual vpn_channel_info * create_channel_server(int client_id, const char * out_ip,  const char * out_ip1, int out_port, const char * in_ip, const char * in_ip1, int in_port, const char * local_ip, int local_port)=0;
    virtual bool send_create_channel_msg(int m_client_id, int m_channel_id) = 0;
};
