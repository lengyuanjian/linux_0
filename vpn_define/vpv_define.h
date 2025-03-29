#pragma once

#define vpn_version (0xf1)


#define  heartbeat              'h'
#define  creace_tcp_channel     't'
#define  client_register        'r'

#pragma pack(push,1)

struct vpn_head
{
    unsigned short                  m_body_len;
    char                            m_type;
    unsigned char                   m_version;
};
struct vpn_heartbeat
{
    vpn_heartbeat()
    {
        m_head.m_body_len = sizeof(*this) - sizeof(vpn_head);
        m_head.m_version = vpn_version;
        m_head.m_type = heartbeat;
    }
    vpn_head                        m_head;
};
struct vpn_client_register
{
    vpn_client_register()
    {
        m_head.m_body_len = sizeof(*this) - sizeof(vpn_head);
        m_head.m_version = vpn_version;
        m_head.m_type = client_register;
    }
    vpn_head                        m_head;
    int                             m_clinet_id;
    char                            m_mac[18];
    char                            m_name[14];
    
};
struct vpn_creace_tcp_channel
{
    vpn_creace_tcp_channel()
    {
        m_head.m_body_len = sizeof(*this) - sizeof(vpn_head);
        m_head.m_version = vpn_version;
        m_head.m_type = creace_tcp_channel;
    }
    vpn_head                        m_head;
    char                            m_server_ip[32];
    int                             m_server_port;
    char                            m_client_ip[32];
    int                             m_client_port;
    int                             m_flag;
};
#include <stdio.h>
inline void log_msg(vpn_head * p_head)
{
    switch (p_head->m_type) 
    {
        case heartbeat:
        {
            printf("Message Type: Heartbeat\n");
            printf("Version: 0x%x\n", p_head->m_version);
            printf("Body Length: %u\n", p_head->m_body_len);
        }
        break;
        case creace_tcp_channel:
        {
            vpn_creace_tcp_channel *msg = reinterpret_cast<vpn_creace_tcp_channel*>(p_head);
            printf("Message Type: Create TCP Channel\n");
            printf("Version: 0x%x\n", msg->m_head.m_version);
            printf("Body Length: %u\n", msg->m_head.m_body_len);
            printf("Server IP: %s\n", msg->m_server_ip);
            printf("Server Port: %d\n", msg->m_server_port);
            printf("Client IP: %s\n", msg->m_client_ip);
            printf("Client Port: %d\n", msg->m_client_port);
            printf("Flag: %d\n", msg->m_flag);
        }
        break;
        case client_register:
        {
            vpn_client_register *msg = reinterpret_cast<vpn_client_register*>(p_head);
            printf("Message Type: Client Register\n");
            printf("Version: 0x%x\n", msg->m_head.m_version);
            printf("Body Length: %u\n", msg->m_head.m_body_len);
            printf("Client ID: %d\n", msg->m_clinet_id);
            printf("MAC: %s\n", msg->m_mac);
            printf("Name: %s\n", msg->m_name);
        }
        break;
        default:
        {
            printf("Unknown Message Type: 0x%x\n", p_head->m_type);
            printf("Version: 0x%x\n", p_head->m_version);
            printf("Body Length: %u\n", p_head->m_body_len);
        }
        break;
    }
}

#pragma pack(pop)
