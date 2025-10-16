#include "interface/vpn_context.h"
#include "business_server.h"
#include <cstdio>
#include <string.h>
// #include "data_channel_server.h"



bool business_server::init(vpn_context * p_vpn_context, const char * ip, int port)
{
    m_p_vpn_context = p_vpn_context;
    if(!m_vpn_server.init(this, p_vpn_context->m_p_log, ip, port))
    {
        return false;
    }
    return true;
}
bool business_server::start()
{
    if(!m_vpn_server.start())
    {
        return false;
    }
    return true;
}
void business_server::stop()
{
    m_vpn_server.stop();
}
void business_server::close()
{
    m_vpn_server.close();
}

void business_server::on_connected(sh_net::socket_info * info)
{
    info->m_data.m_ptr = new vpn_client_info;
    vpn_client_info * p_client_info = (vpn_client_info *)info->m_data.m_ptr;
    int client_id = (int)(info->m_sh_fd);
    m_clinet_map.insert({client_id, p_client_info});
    p_client_info->m_clinet_id = client_id;
    p_client_info->m_info = info;
}
void business_server::on_disconnected(sh_net::socket_info * info)
{
    vpn_client_info * p_client_info = (vpn_client_info *)info->m_data.m_ptr;
    
    auto it = m_clinet_map.find(info->m_sh_fd);
    if(it != m_clinet_map.end())
    {
        for(auto & it_channel : it->second->m_channel_map)
        {
            m_p_vpn_context->m_p_data_channel_server->delete_channel_server(it->second->m_clinet_id, it_channel.second->m_channel_id);
            delete it_channel.second;
        }
        m_clinet_map.erase(it);
    }
    
    if(p_client_info)
    {
        delete p_client_info;
        p_client_info = nullptr;
    }
    
    info->m_data.m_ptr = nullptr;
}
void business_server::on_a_disconnected(sh_net::socket_info * info)
{
    on_disconnected(info);
}
int  business_server::on_recv_data(sh_net::socket_info * info)
{
    int deal_len = 0;
    char * buff = info->m_data.m_buff;
    int buff_len = info->m_data.m_buff_len;
    auto need_len = [](vpn_head * p_head)
    {
        return p_head->m_body_len + sizeof(vpn_head);
    };
    {
        while(true)
        {
            if(buff_len >= (int)sizeof(vpn_head))
            {
                vpn_head * p_head = (vpn_head *)buff;
                int n_len = need_len(p_head);
                if(buff_len >= n_len)
                {
                    deal_data(info, p_head, buff);
                    deal_len += n_len;
                    buff     += n_len;
                    buff_len -= n_len;
                }
            }
            else 
            {
                return deal_len;
            }
        }
    }
}

void business_server::send_msg_tcp_channel(int fd, const char * server_ip, int server_port, const char * local_ip, int local_port)
{
    vpn_creace_tcp_channel msg;
    strcpy(msg.m_server_ip, server_ip);
    msg.m_server_port = server_port;
    strcpy(msg.m_client_ip, local_ip);
    msg.m_client_port = local_port;
    msg.m_flag = 0;
    log_msg((vpn_head*)(&msg));
    ::send(fd, &msg, sizeof(msg),0);
}

bool business_server::send_create_channel_msg(int m_client_id, int m_channel_id)
{
    auto it = m_clinet_map.find(m_client_id);
    if(it != m_clinet_map.end())
    {
        auto it1 = it->second->m_channel_map.find(m_channel_id);
        if(it1 != it->second->m_channel_map.end())
        {
            vpn_channel_info * channel_info = it1->second;
            send_msg_tcp_channel(it->second->m_info->m_fd, channel_info->m_in_ip1, channel_info->m_in_port, channel_info->m_local_ip, channel_info->m_local_port);
            return true;
        }
        else 
        {
            m_p_vpn_context->m_p_log->on_error(stderr, "not find channel[%d][%d]",m_client_id, m_channel_id);
        }
    }
    else
    {
        m_p_vpn_context->m_p_log->on_error(stderr, "not find m_client[%d][%d]",m_client_id, m_channel_id);
    }
    return false;
}


void business_server::deal_client_register(sh_net::socket_info * info, vpn_client_register * p_msg)
{
    vpn_client_info * client_info = (vpn_client_info *)(info->m_data.m_ptr);
    if(client_info == nullptr)
    {
        return;
    }
    if(p_msg->m_clinet_id == 0)
    {
        p_msg->m_clinet_id = client_info->m_clinet_id;
    }
    else
    {
        p_msg->m_clinet_id = client_info->m_clinet_id;
    }
    ::send(info->m_fd, p_msg, sizeof(vpn_client_register),0);
}

void business_server::deal_data(sh_net::socket_info * info, vpn_head * p_head, char * buff)
{
    log_msg(p_head);
    switch (p_head->m_type)
    {
        case heartbeat:
        {
            printf("heartbeat\n");
        }
        break;
        case creace_tcp_channel:
        {
            //vpn_creace_tcp_channel * p_creace_tcp_channel = (vpn_creace_tcp_channel *)buff;
            // wait_create_tcp_channel(p_creace_tcp_channel);
        }
        break;
        case client_register:
        {
            vpn_client_register * p_msg = (vpn_client_register *)buff;
            deal_client_register(info, p_msg);
        }
        break;
    }
}


vpn_channel_info * business_server::create_channel_server(int client_id, const char * out_ip, const char * out_ip1, int out_port, const char * in_ip, const char * in_ip1, int in_port, const char * local_ip, int local_port)
{
    auto it = m_clinet_map.find(client_id);
    if(it != m_clinet_map.end())
    {
        int max_channel_id = 0;
        for(auto & it_channel : it->second->m_channel_map)
        {
            if(it_channel.second->m_channel_id > max_channel_id)
            {
                max_channel_id = it_channel.second->m_channel_id;
            }
        }
        max_channel_id++;
        vpn_channel_info * channel_info = new vpn_channel_info;
        channel_info->m_client_id = client_id;
        channel_info->m_channel_id = max_channel_id;
        strcpy(channel_info->m_out_ip, out_ip);
        strcpy(channel_info->m_out_ip1, out_ip1);
        channel_info->m_out_port = out_port;
        strcpy(channel_info->m_in_ip, in_ip);
        strcpy(channel_info->m_in_ip1, in_ip1);
        channel_info->m_in_port = in_port;
        strcpy(channel_info->m_local_ip, local_ip);
        channel_info->m_local_port = local_port;
        
        if(!m_p_vpn_context->m_p_data_channel_server->create_channel_server(channel_info))
        {
            delete channel_info;
            m_p_vpn_context->m_p_log->on_error(stderr, "create channel failed [%d][%d] out_ip[%s] out_port[%d] in_ip[%s] in_port[%d] local_ip[%s] local_port[%d]", client_id, max_channel_id, out_ip, out_port, in_ip, in_port, local_ip, local_port);
            return nullptr;
        }
        m_p_vpn_context->m_p_log->on_info(stdout, "create channel secc [%d][%d] out_ip[%s] out_port[%d] in_ip[%s] in_port[%d] local_ip[%s] local_port[%d]", client_id, max_channel_id, out_ip, out_port, in_ip, in_port, local_ip, local_port);
        
        
        it->second->m_channel_map.insert({max_channel_id, channel_info});
        
        return channel_info;
    }
    else 
    {
        m_p_vpn_context->m_p_log->on_error(stderr, "not find client[%d]", client_id);
        return nullptr;
    }
    return nullptr;
}