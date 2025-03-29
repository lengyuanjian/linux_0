#include "s_vpn.h"
#include "s_vpn_server.h"
#include <cstdio>
#include <cstring>

void vpn_server::printf_client_channel()
{
    for(auto [id,client_info] : m_vpn_client_map)
    {
        printf("id[%d]client[%d]:[%s:%d]->[%s:%d]\n",id, client_info->m_clinet_id
            , client_info->m_info->m_local_ip, client_info->m_info->m_local_port
            , client_info->m_info->m_remve_ip, client_info->m_info->m_remve_port);
        for(auto [c_id, channel_info]: client_info->m_channel_map)
        {
            printf("  -channel[%d][%s:%d]<->[%s:%d]<->[%s:%d]\n",c_id
                ,channel_info->m_out_ip,channel_info->m_out_port
                ,channel_info->m_in_ip,channel_info->m_in_port
                ,channel_info->m_local_ip,channel_info->m_local_port);
        }
    }
}

void vpn_server::add_client(socket_info * info)
{
    vpn_client_info * client_info = new vpn_client_info;
    client_info->m_info = info;
    client_info->m_clinet_id = info->m_sh_fd;
    m_vpn_client_map.insert({info->m_sh_fd, client_info});
    info->m_data.m_ptr = client_info;
}
void vpn_server::del_client(socket_info * info)
{
    auto it = m_vpn_client_map.find(info->m_sh_fd);
    if(it != m_vpn_client_map.end())
    {
        for(auto & [_, channel_info] : it->second->m_channel_map)
        {
            channel_info->m_p_channel->close();
            delete channel_info->m_p_channel;
            delete channel_info;
        }
        delete it->second;
        m_vpn_client_map.erase(it);
    }    
}

vpn_channel_info * vpn_server::add_channel(sh_fd client_id, const char * out_ip, int out_port, const char * in_ip, int in_port, const char * local_ip, int local_port)
{
    vpn_channel_info * p_channel_info = new vpn_channel_info;
    auto it = m_vpn_client_map.find(client_id);
    if(it == m_vpn_client_map.end())
    {
        m_p_log->on_error(stderr,"client id not find[%d]",client_id);
        return nullptr;
    }
    p_channel_info->m_channel_id = m_channel_id++;
    strcpy(p_channel_info->m_out_ip,out_ip);
    p_channel_info->m_out_port = out_port;
    strcpy(p_channel_info->m_in_ip,in_ip);
    p_channel_info->m_in_port = in_port;
    strcpy(p_channel_info->m_local_ip,local_ip);
    p_channel_info->m_local_port = local_port;
    p_channel_info->m_p_channel = new vpn_server_oi();
    it->second->m_channel_map.insert({p_channel_info->m_channel_id, p_channel_info});

    return p_channel_info;
}

void vpn_server::on_connected(socket_info * info)
{
    add_client(info);
}
void vpn_server::on_disconnected(socket_info * info)
{
    del_client(info);
}
void vpn_server::on_a_disconnected(socket_info * info)
{
    del_client(info);
}
void vpn_server::deal_data(socket_info * info, vpn_head * p_head, char * buff)
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
            vpn_creace_tcp_channel * p_creace_tcp_channel = (vpn_creace_tcp_channel *)buff;
            wait_create_tcp_channel(p_creace_tcp_channel);
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
void vpn_server::wait_create_tcp_channel(vpn_creace_tcp_channel * p_creace_tcp_channel)
{
    
}
void vpn_server::deal_client_register(socket_info * info, vpn_client_register * p_msg)
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
int  vpn_server::on_recv_data(socket_info * info)
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
bool vpn_server::send_create_channel_msg(int m_client_id, int m_channel_id)
{
    auto it = m_vpn_client_map.find(m_client_id);
    if(it != m_vpn_client_map.end())
    {
        auto it1 = it->second->m_channel_map.find(m_channel_id);
        if(it1 != it->second->m_channel_map.end())
        {
            vpn_channel_info * channel_info = it1->second;
            send_msg_tcp_channel(it->second->m_info->m_fd, channel_info->m_in_ip, channel_info->m_in_port, channel_info->m_local_ip, channel_info->m_local_port);
            return true;
        }
        else 
        {
            m_p_log->on_error(stderr, "not find channel[%d][%d]",m_client_id, m_channel_id);
        }
    }
    else
    {
        m_p_log->on_error(stderr, "not find m_client[%d][%d]",m_client_id, m_channel_id);
    }
    return false;
}
void vpn_server::send_msg_tcp_channel(int fd, const char * server_ip, int server_port, const char * local_ip, int local_port)
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

bool vpn_server::init(log_event * p_log,const char * ip, int port)
{
    m_p_log = p_log;
    if(!m_vpn_server.init(this, p_log, ip, port))
    {
        m_p_log->on_error(stderr,"init failed");
        return false;
    }
    
    return true;
}
bool vpn_server::start()
{
    if(!m_vpn_server.start())
    {
        m_p_log->on_error(stderr,"start failed");
        return false;
    }
    return true;
}
void vpn_server::stop()
{
    m_vpn_server.stop();
}
void vpn_server::close()
{
    m_vpn_server.close();
}

bool vpn_server::create_channel_server(sh_fd client_id, const char * out_ip, int out_port, const char * in_ip, int in_port, const char * local_ip, int local_port)
{
    vpn_channel_info * channel_info = add_channel(client_id, out_ip, out_port, in_ip, in_port, local_ip, local_port);
    if(channel_info == nullptr)
    {
        return false;
    }
    bool ret = channel_info->m_p_channel->init(m_p_log, this, client_id, channel_info->m_channel_id, out_ip, out_port, in_ip, in_port);
    if(!ret)
    {
        m_p_log->on_error(stderr, "channel init failed!");
        
    }
    return ret;
}
