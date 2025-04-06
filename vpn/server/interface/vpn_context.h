#pragma once
#include "i_business_server.h"
#include "i_data_channel_server.h"
class log_event;


struct vpn_context
{
    log_event *                 m_p_log;
    i_business_server *         m_p_business_server;
    i_data_channel_server *     m_p_data_channel_server;
};