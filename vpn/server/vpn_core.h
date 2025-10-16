#pragma once
#include "interface/vpn_context.h"

class vpn_core
{
    vpn_context * m_p_context{nullptr};
public:
    ~vpn_core(){};
    bool init(const char * ip, int port);
    bool start();
    void exec();
    void stop();
    void close();
};
