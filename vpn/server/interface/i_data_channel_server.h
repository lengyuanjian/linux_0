#pragma once
#include "vpn_define.h"


class vpn_context;
class i_data_channel_server
{
public:
    virtual ~i_data_channel_server(){}
    virtual bool init(vpn_context * p_vpn_context) = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void close() = 0;

public:
    virtual bool create_channel_server(vpn_channel_info * p_channel_info) = 0;
    virtual bool delete_channel_server(int client_id, int channel_id) = 0;
};