#include "s_vpn_server.h"
#include "s_vpn.h"
using namespace sh_net;

class event_out:public sh_net::socket_event
{
    vpn_server_oi * m_p_context{nullptr};
public:
    virtual ~event_out(){}
    void init(vpn_server_oi * p_context)
    {
        m_p_context = p_context;
    }
    virtual void on_accept(sh_net::socket_info * info)
    {
        m_p_context->get_log()->on_debug(stdout,"on accept [%u] out begin",info->m_sh_fd);
        {
            std::lock_guard<std::mutex> lock(m_p_context->get_mutex());
            m_p_context->set_socket_info(info);
        }
        
        // 发送消息
        m_p_context->notify_create_channel();
        // 
        m_p_context->get_cond().notify_one();
        // // 等待连接
        // {
        //     std::unique_lock<std::mutex> unique_lock(m_p_context->get_mutex());
        //     m_p_context->get_cond().wait_for(unique_lock, std::chrono::milliseconds(5000));
        // }
        m_p_context->get_log()->on_debug(stdout,"on accept [%u] out end",info->m_sh_fd);
    }
    virtual void on_connected(sh_net::socket_info * info)
    {
        m_p_context->get_log()->on_debug(stdout,"on connected [%u] out begin",info->m_sh_fd);
        {
            std::lock_guard<std::mutex> lock(m_p_context->get_connected_mutex());
            m_p_context->add_connected();
        }
        m_p_context->get_connected_cond().notify_one();
        m_p_context->get_log()->on_debug(stdout,"on connected [%u] out end",info->m_sh_fd);
    }
    virtual void on_disconnected(sh_net::socket_info * info)
    {
        if(info->m_data.m_ptr) 
        {
            m_p_context->get_log()->on_debug(stdout,"on a disconnected [%u] out close[%u]",info->m_sh_fd, ((socket_info *)(info->m_data.m_ptr))->m_sh_fd);
            close_socket((socket_info *)(info->m_data.m_ptr));
        }
        else
        {
            m_p_context->get_log()->on_debug(stdout,"on a disconnected [%u] out nullptr",info->m_sh_fd);
        }
    }
    virtual void on_a_disconnected(sh_net::socket_info * info)
    {
        // if(info->m_data.m_ptr) 
        // {
        //     m_p_context->get_log()->on_debug(stdout,"on a disconnected [%u] out a close[%u]",info->m_sh_fd, ((socket_info *)(info->m_data.m_ptr))->m_sh_fd);
        //     close_socket((socket_info *)(info->m_data.m_ptr));
        // }
        // else
        // {
        //     m_p_context->get_log()->on_debug(stdout,"on a disconnected [%u] out nullptr",info->m_sh_fd);
        // }
    }
    virtual int on_recv_data(sh_net::socket_info * info)
    {
        m_p_context->get_log()->on_debug(stdout,"rx:[%u][%d]ptr[%x]",info->m_sh_fd,info->m_data.m_buff_len,info->m_data.m_ptr);
        if(info->m_data.m_ptr) 
        {
            socket_info * p_remote_clinet_info = (socket_info *)info->m_data.m_ptr;
            ::send(p_remote_clinet_info->m_fd, info->m_data.m_buff, info->m_data.m_buff_len,0);
        }
        else
        {

        }
        return info->m_data.m_buff_len;
    }
};

class event_in:public sh_net::socket_event
{
    vpn_server_oi * m_p_context{nullptr};
public:
    virtual ~event_in(){}
    void init(vpn_server_oi * p_context)
    {
        m_p_context = p_context;
    }
    virtual void on_accept(sh_net::socket_info * info)
    {
        m_p_context->get_log()->on_debug(stdout,"on accept [%u] in begin",info->m_sh_fd);
        socket_info * remote_info;
        {
            std::unique_lock<std::mutex> unique_lock(m_p_context->get_mutex());
            remote_info = m_p_context->get_socket_info();
            if(remote_info == nullptr)
            {
                m_p_context->get_cond().wait_for(unique_lock,std::chrono::milliseconds(5000));
                remote_info = m_p_context->get_socket_info();
            }
        }
        
        if(remote_info == nullptr)
        {
            m_p_context->get_log()->on_debug(stdout,"on accept [%u] in nullptr",info->m_sh_fd);
            return;
        }
        {
            std::lock_guard<std::mutex> lock(m_p_context->get_mutex());
            info->m_data.m_ptr = remote_info;
            remote_info->m_data.m_ptr = info;
            m_p_context->set_socket_info(nullptr);
        }
        // m_p_context->get_cond().notify_one();
        {
            std::unique_lock<std::mutex> unique_lock(m_p_context->get_connected_mutex());
            if(m_p_context->is_connected())
            {
                m_p_context->sub_connected();
            }
            else 
            {
                m_p_context->get_cond().wait_for(unique_lock,std::chrono::milliseconds(5000));
                m_p_context->sub_connected();
            }
            
        }
        m_p_context->get_log()->on_debug(stdout,"on accept [%u] in end",info->m_sh_fd);
               
    }
    virtual void on_connected(sh_net::socket_info * info)
    {
        // m_p_context->get_log()->on_debug(stdout,"on connected [%u] in begin",info->m_sh_fd);
        
        // m_p_context->get_log()->on_debug(stdout,"on connected [%u] in end",info->m_sh_fd);
    }
    virtual void on_disconnected(sh_net::socket_info * info)
    {
        if(info->m_data.m_ptr) 
        {
            m_p_context->get_log()->on_debug(stdout,"on a disconnected [%u] in close[%u]",info->m_sh_fd, ((socket_info *)(info->m_data.m_ptr))->m_sh_fd);
            close_socket((socket_info *)(info->m_data.m_ptr));
        }
        else
        {
            m_p_context->get_log()->on_debug(stdout,"on disconnected [%u] in nullptr",info->m_sh_fd);
        }
    }
    virtual void on_a_disconnected(sh_net::socket_info * info)
    {
        // if(info->m_data.m_ptr) 
        // {
        //     m_p_context->get_log()->on_debug(stdout,"on a disconnected [%u] in a close[%u]",info->m_sh_fd, ((socket_info *)(info->m_data.m_ptr))->m_sh_fd);
        //     close_socket((socket_info *)(info->m_data.m_ptr));
        // }
        // else
        // {
        //     m_p_context->get_log()->on_debug(stdout,"on a disconnected [%u] in nullptr",info->m_sh_fd);
        // }
    }
    virtual int on_recv_data(sh_net::socket_info * info)
    {
        m_p_context->get_log()->on_debug(stdout,"rx:[%d]ptr[%x]",info->m_data.m_buff_len,info->m_data.m_ptr);
        if(info->m_data.m_ptr) 
        {
            socket_info * p_remote_clinet_info = (socket_info *)info->m_data.m_ptr;
            ::send(p_remote_clinet_info->m_fd, info->m_data.m_buff, info->m_data.m_buff_len,0);
        }
        else
        {
            
        }
        return info->m_data.m_buff_len;
    }
};

bool vpn_server_oi::init(log_event * p_log, vpn_server * p_vpn_server, int client_id, int channel_id, const char * out_ip, int out_port, const char * in_ip, int in_port)
{
    m_p_log = p_log;
    m_client_id = client_id;
    m_channel_id = channel_id;
    m_p_event_in = new event_in();
    m_p_event_out = new event_out();
    m_p_event_in->init(this);
    m_p_event_out->init(this);
    m_p_vpn_server = p_vpn_server;
    if(!m_out_server.init(m_p_event_out, p_log, out_ip, out_port))
    {
        return false;
    }
    if(!m_in_server.init(m_p_event_in, p_log, in_ip, in_port))
    {
        return false;
    }

    if(!m_out_server.start())
    {
        return false;
    }
    if(!m_in_server.start())
    {
        return false;
    }
    return true;
}

void vpn_server_oi::notify_create_channel()
{
    m_p_vpn_server->send_create_channel_msg(m_client_id, m_channel_id);
}

void vpn_server_oi::close()
{
    m_cond.notify_all();
    m_out_server.stop();
    m_in_server.stop();
    m_out_server.close();
    m_in_server.close();
    delete  m_p_event_in;
    delete  m_p_event_out;
}

