#pragma once

struct vpn_channel_info
{
    int             m_client_id{0};
    int             m_channel_id{0};
    char            m_out_ip[32]{};
    int             m_out_port{0};
    char            m_in_ip[32]{};
    int             m_in_port{0};  
    char            m_local_ip[32]{};
    int             m_local_port{0};
};