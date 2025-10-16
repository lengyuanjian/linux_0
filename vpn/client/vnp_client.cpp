#include "../com_lib/protocol/vpv_protocol.h"
#include "../com_lib/net_lib/sh_socket_client.h"
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>

using namespace sh_net;

class main_client_event:public sh_net::socket_event
{
    bool check_head(vpn_head * p_head)
    {
        if(p_head->m_version != vpn_version)
        {
            return false;
        }
        switch (p_head->m_type)
        {
            case heartbeat:
            case creace_tcp_channel:
            case client_register:
            return true;
            default: return false; 
        }
    }
    int need_len(vpn_head * p_head)
    {
        return p_head->m_body_len + sizeof(vpn_head);
    }
    void wait_create_tcp_channel(socket_info * info, vpn_creace_tcp_channel * p_creace_tcp_channel)
    {
        socket_info * s_info = m_p_server_client->connect(p_creace_tcp_channel->m_server_ip, p_creace_tcp_channel->m_server_port);
        socket_info * l_info = m_p_local_client->connect(p_creace_tcp_channel->m_client_ip, p_creace_tcp_channel->m_client_port);
        if(s_info != nullptr && l_info != nullptr)
        {
            s_info->m_data.m_ptr = l_info;
            l_info->m_data.m_ptr = s_info;

            p_creace_tcp_channel->m_flag = 1;
            ::send(info->m_fd, p_creace_tcp_channel, sizeof(vpn_creace_tcp_channel),0);
            
            m_p_server_client->add_eool(s_info);
            m_p_local_client->add_eool(l_info);
        }
        else 
        {
            if(s_info)
            {
                m_p_server_client->add_eool(s_info);
                m_p_server_client->close(s_info);
            }
            if(l_info)
            {
                m_p_local_client->add_eool(l_info);
                m_p_local_client->close(l_info);
            }
            p_creace_tcp_channel->m_flag = 0;
            ::send(info->m_fd, p_creace_tcp_channel, sizeof(vpn_creace_tcp_channel),0);
        }
    }
    void deal_data(socket_info * info, vpn_head * p_head, char * buff)
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
                vpn_creace_tcp_channel * msg = (vpn_creace_tcp_channel *)buff;
                wait_create_tcp_channel(info,msg);
            }
            break;
            case client_register:
            {
                vpn_client_register * p_client_register = (vpn_client_register *)buff;
                printf("client_register id[%d]\n",p_client_register->m_clinet_id);
            }
            break;
        }
    }
protected:
    void set_msg_client_register(vpn_client_register & msg)
    {
        msg.m_clinet_id = 0;
        strcpy(msg.m_mac,"A8-5E-45-D5-0A-9A");
        strcpy(msg.m_name,"vpn_client");
    }
private:
    sh_net::socket_client * m_p_server_client;
    sh_net::socket_client * m_p_local_client;
public:
    virtual ~main_client_event(){}
    void init(sh_net::socket_client * p_server, sh_net::socket_client *p_local)
    {
        m_p_server_client = p_server;
        m_p_local_client = p_local;
    };
    virtual void on_connected(socket_info * info)
    {
        vpn_client_register  client_register_msg;
        set_msg_client_register(client_register_msg);
        ::send(info->m_fd, &client_register_msg, sizeof(vpn_client_register),0);
    }
    virtual void on_disconnected(socket_info * info)
    {
        std::condition_variable * p_re_connect_cv = (std::condition_variable *)info->m_data.m_ptr;
        p_re_connect_cv->notify_one();
    }
    virtual void on_a_disconnected(socket_info * info)
    {
        (void)info;
    }
    virtual int on_recv_data(socket_info * info)
    {
        int deal_len = 0;
        char * buff = info->m_data.m_buff;
        int buff_len = info->m_data.m_buff_len;
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
};



class vpn_client_event:public sh_net::socket_event
{
public:
    virtual ~vpn_client_event(){}
    
    virtual void on_connected(socket_info * info)
    {
        (void)info;
    }
    virtual void on_disconnected(socket_info * info)
    {
        if(info->m_data.m_ptr) 
        {
            socket_info * p_remote_clinet_info = (socket_info *)info->m_data.m_ptr;
            p_remote_clinet_info->m_data.m_ptr = nullptr;
            close_socket(p_remote_clinet_info);
        }
    }
    virtual void on_a_disconnected(socket_info * info)
    {
        (void)info;
        // if(info->m_data.m_ptr) 
        // {
        //     socket_info * p_remote_clinet_info = (socket_info *)info->m_data.m_ptr;
        //     p_remote_clinet_info->m_data.m_ptr = nullptr;
        //     close_socket(p_remote_clinet_info);
        // }
    }
    virtual int on_recv_data(socket_info * info)
    {
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

class vnp_client
{
    sh_net::socket_client    m_main_client;
    main_client_event        m_main_client_event;
    
    sh_net::socket_client    m_server_client;
    sh_net::socket_client    m_local_client;
    vpn_client_event         m_server_client_event;
    log_event *              m_p_log;
    int                      m_clinet_id;
    std::string              m_server_ip;
    int                      m_server_port;
    bool                     m_re_connect{true};
    std::thread              m_re_connect_thread;
    std::mutex               m_re_connect_mutex;
    std::condition_variable  m_re_connect_cv;
public:
    virtual ~vnp_client(){}
    bool init(log_event * p_log, const char * server_ip, int server_port)
    {
        m_p_log = p_log;
        m_server_ip = server_ip;
        m_server_port = server_port;
        m_main_client_event.init(&m_server_client,&m_local_client);
        if(!m_main_client.init(&m_main_client_event,p_log))
        {
            m_p_log->on_error(stderr,"init failed");
            return false;
        }

        if(!m_server_client.init(&m_server_client_event,p_log))
        {
            m_p_log->on_error(stderr,"init failed");
            return false;
        }
        if(!m_local_client.init(&m_server_client_event,p_log))
        {
            m_p_log->on_error(stderr,"init failed");
            return false;
        }
        return true;
    }
    bool start()
    {
        if (!m_server_client.start()) 
        {
            m_p_log->on_error(stderr,"start failed");
            return false;
        
        }
        if(!m_local_client.start())
        {
            m_p_log->on_error(stderr,"start failed");
            return false;
        }
        if(!m_main_client.start())
        {
            m_p_log->on_error(stderr,"start failed");
            return false;
        }
        connect_server();
        m_re_connect_thread = std::thread(&vnp_client::re_connect, this);
        return true;
    }
    void connect_server()
    {
        socket_info* info = nullptr;
        while(true)
        {
            info = m_main_client.connect(m_server_ip.c_str(), m_server_port);
            if(info)
            {
                break;
            }
            else 
            {
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
        info->m_data.m_ptr = &m_re_connect_cv;
        m_main_client.add_eool(info);
    }
    void re_connect()
    {
        while(m_re_connect)
        {
            std::unique_lock<std::mutex> lock(m_re_connect_mutex);
            m_re_connect_cv.wait(lock);
            connect_server();
        }
    }
    void stop()
    {
        m_re_connect = false;
        m_re_connect_cv.notify_all();
        m_re_connect_thread.join();
        m_main_client.stop();
        m_server_client.stop();
        m_local_client.stop();
    }
    void close()
    {
        m_main_client.close();
        m_server_client.close();
        m_local_client.close();
    }
};
#include <iostream>

int main(int argc, char * argv[]) 
{
    if(argc < 3)
    {
        std::cout<<"./exec ip port\n";
        return 1;
    }

    const char * ip = argv[1];
    int port = std::atoi(argv[2]); 
    log_event log;
    vnp_client app;
    if(!app.init(&log,ip, port))
    {
        return 1;
    }
    if(!app.start())
    {
        return 1;
    }
    std::string input;
    while (true) 
    {
        std::cout << "Enter 'q' to quit: ";
        std::getline(std::cin, input);
        if (input == "q") 
        {
            break;
        }
    }
    app.stop();
    return 0;
}
