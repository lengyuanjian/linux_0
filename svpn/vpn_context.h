#pragma once

#include <vector>
#include <string>

class vpn_server;
class log_event;

struct vpn_context 
{
    vpn_server * m_p_vpn_server{nullptr};
    log_event * m_p_log{nullptr};
};  

class vpn_core
{
    vpn_context * m_p_context{nullptr};
public:
    vpn_core(){};
    ~vpn_core(){};
    bool init(const char * ip, int port);
    bool start();
    void exec();
    void stop();
    void close();
};