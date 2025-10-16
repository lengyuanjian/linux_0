#include "data_channel_server.h"
#include "interface/vpn_define.h"
#include <cstdio>
#include <string.h>
#include <sys/epoll.h>

bool data_channel_server::init(vpn_context * p_vpn_context)
{
    m_p_vpn_context = p_vpn_context;
    if(!m_vpn_data_socket.init(this, p_vpn_context->m_p_log))
    {
        return false;
    }
    return true;
}
bool data_channel_server::start()
{
    if(!m_vpn_data_socket.start())
    {
        return false;
    }
    m_accept_thread = std::thread(&data_channel_server::run_accept, this);
    return true;
}
void data_channel_server::stop()
{
    m_vpn_data_socket.stop();
}
void data_channel_server::close()
{
    m_vpn_data_socket.close();
}

int data_channel_server::server_sock_init(const char *ip, int port)
{
    int server_fd = 0;
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        m_p_vpn_context->m_p_log->on_error(stderr,"socket create failed");
        return -1;
    }
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        m_p_vpn_context->m_p_log->on_error(stderr,"socket setsockopt failed!");
        ::close(server_fd);
        return -1;
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;   
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons((unsigned short)port);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        m_p_vpn_context->m_p_log->on_error(stderr,"socket bind failed![%s:%d]",ip,port);
        ::close(server_fd);
        return -1;
    }
    m_p_vpn_context->m_p_log->on_debug(stdout,"server bind[%s:%d]",ip,port);
    return server_fd; 
}

server_socket_info * data_channel_server::init_server_socket(const char * out_ip, int out_port,const char * in_ip, int in_port)
{
    int out_socket = server_sock_init(out_ip, out_port);
    if(out_socket == -1)
    {
        return nullptr;
    }
    int in_socket = server_sock_init(in_ip, in_port);
    if(in_socket == -1)
    {
        ::close(out_socket);
        return nullptr;
    }
    
    server_socket_info * out_server_info = new server_socket_info;
    server_socket_info * in_server_info = new server_socket_info;
    
    out_server_info->m_p_removte = in_server_info;
    strcpy(out_server_info->m_local_ip, out_ip);
    out_server_info->m_local_port = out_port;
    out_server_info->m_fd = out_socket;
    out_server_info->m_type = 1;


    in_server_info->m_p_removte = out_server_info;
    strcpy(in_server_info->m_local_ip, in_ip);
    in_server_info->m_local_port = in_port;
    in_server_info->m_fd = in_socket;
    
    return in_server_info;
}
bool data_channel_server::server_add_eool(server_socket_info * server_info)
{
    if (listen(server_info->m_fd, 10) < 0) 
    {
        ::close(server_info->m_fd);
        m_p_vpn_context->m_p_log->on_error(stderr,"listen failed[%s:%d]",server_info->m_local_ip,server_info->m_local_port);
        return false;
    }
    m_p_vpn_context->m_p_log->on_info(stdout,"server listen[%s:%d]",server_info->m_local_ip,server_info->m_local_port);  
    {
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.ptr = server_info;
        server_info->m_epfd = m_accept_epfd;
        epoll_ctl(m_accept_epfd, EPOLL_CTL_ADD, server_info->m_fd, &ev);
    }
    return true;
}
void data_channel_server::server_delete_eool(server_socket_info * server_info)
{
    epoll_ctl(m_accept_epfd, EPOLL_CTL_DEL, server_info->m_fd, nullptr);
    ::close(server_info->m_fd);
}

void data_channel_server::date_socket_accept(server_socket_info * server_info, sh_net::socket_info * info)
{
    bool is_out = server_info->m_type == 1;
    if(is_out)
    {
        m_p_vpn_context->m_p_business_server->send_create_channel_msg(server_info->m_client_id, server_info->m_channel_id);
    }
    if(server_info->m_p_removte->m_free_socket.empty())
    {
        server_info->m_free_socket.push(info);
    }
    else
    {
        sh_net::socket_info * re_info = server_info->m_p_removte->m_free_socket.front();
        server_info->m_p_removte->m_free_socket.pop();
        info->m_data.m_ptr = re_info;
        re_info->m_data.m_ptr = info;
        server_info->m_busy_socket.push(info);
        server_info->m_p_removte->m_busy_socket.push(re_info);
        if(is_out)
        {
            m_vpn_data_socket.add_eool(re_info);
            m_vpn_data_socket.add_eool(info);
        }
        else 
        {
            m_vpn_data_socket.add_eool(info);
            m_vpn_data_socket.add_eool(re_info);
        }
        
    }
}

bool data_channel_server::create_channel_server(vpn_channel_info * p_channel_info)
{
    server_socket_info * server_info = init_server_socket(p_channel_info->m_out_ip, p_channel_info->m_out_port,p_channel_info->m_in_ip, p_channel_info->m_in_port);
    if(server_info == nullptr)
    {
        m_p_vpn_context->m_p_log->on_error(stderr, "create channel server failed [%d][%d] out_ip[%s] out_port[%d] in_ip[%s] in_port[%d] local_ip[%s] local_port[%d]", p_channel_info->m_client_id, p_channel_info->m_channel_id, p_channel_info->m_out_ip, p_channel_info->m_out_port, p_channel_info->m_in_ip, p_channel_info->m_in_port, p_channel_info->m_local_ip, p_channel_info->m_local_port);
        return false;
    }
    
    server_info->m_client_id = p_channel_info->m_client_id;
    server_info->m_channel_id = p_channel_info->m_channel_id;
    server_info->m_p_removte->m_client_id = p_channel_info->m_client_id;
    server_info->m_p_removte->m_channel_id = p_channel_info->m_channel_id;
    long long key = ((long long)p_channel_info->m_client_id << 32) + p_channel_info->m_channel_id;
    if(!server_add_eool(server_info->m_p_removte) || !server_add_eool(server_info))
    {
        delete server_info;
        delete server_info->m_p_removte;
        return false;
    }
    m_channel_map.insert({key, server_info});
    return true;
}
bool data_channel_server::delete_channel_server(int client_id, int channel_id)
{
    long long key = ((long long)client_id << 32) + channel_id;
    auto it = m_channel_map.find(key);
    if(it == m_channel_map.end())
    {
        m_p_vpn_context->m_p_log->on_error(stderr, "not find channel server [%d][%d]", client_id, channel_id);
        return false;
    }
    server_socket_info * server_info = it->second;
    server_delete_eool(server_info);
    server_delete_eool(server_info->m_p_removte);
    auto close_queue = [](std::queue<sh_net::socket_info *> & queue)
    {
        while(!queue.empty())
        {
            sh_net::socket_info * info = queue.front();
            queue.pop();
            sh_net::close_socket(info);
        }
    };
    close_queue(server_info->m_free_socket);
    close_queue(server_info->m_busy_socket);
    close_queue(server_info->m_p_removte->m_free_socket);
    close_queue(server_info->m_p_removte->m_busy_socket);
    
    return true;
}

void data_channel_server::on_connected(sh_net::socket_info * info)
{
    (void)info;
}
void data_channel_server::on_disconnected(sh_net::socket_info * info)
{
    sh_net::socket_info * remote_info = (sh_net::socket_info *)info->m_data.m_ptr;
    if(remote_info != nullptr)
    {
        remote_info->m_data.m_ptr = nullptr;
        sh_net::close_socket(remote_info);
    }
}
void data_channel_server::on_a_disconnected(sh_net::socket_info * info)
{
    (void)info;
    // sh_net::socket_info * remote_info = (sh_net::socket_info *)info->m_data.m_ptr;
    // if(remote_info != nullptr)
    // {
    //     remote_info->m_data.m_ptr = nullptr;
    //     sh_net::close_socket(remote_info);
    // }
    // else
    // {
    
    // }
}
int  data_channel_server::on_recv_data(sh_net::socket_info * info)
{
    sh_net::socket_info * remote_info = (sh_net::socket_info *)info->m_data.m_ptr;
    m_p_vpn_context->m_p_log->on_info(stdout, sh_net_socket_format "recv data len:%d[%p]" , sh_net_socket_args(info), info->m_data.m_buff_len,remote_info);
    if(remote_info == nullptr)
    {
        sh_net::close_socket(info);
        return info->m_data.m_buff_len;
    }
    else 
    {
        int len = ::send(remote_info->m_fd, info->m_data.m_buff, info->m_data.m_buff_len, 0);
        if(len > 0)
        {
            return len;
        }
        else 
        {
            m_p_vpn_context->m_p_log->on_error(stderr, "send data failed error[%d]", errno);
        }
        return info->m_data.m_buff_len;
    }
}

void data_channel_server::run_accept()
    {
        m_accept_epfd = epoll_create(1);
        epoll_event evs[1024] = {};
        while(m_accept_run)
        {
            int nready = epoll_wait(m_accept_epfd, evs, 1024, 1000);
            for(int i = 0; i < nready; ++i)
            {
                server_socket_info * server_info = (server_socket_info *)(evs[i].data.ptr);
                int server_fd = server_info->m_fd;
                if(evs[i].events & EPOLLIN)
                {
                    int new_socket;
                    sockaddr_in address;
                    int addrlen = sizeof(sockaddr_in);
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
                    {
                        m_p_vpn_context->m_p_log->on_error(stderr,"accept error! [%s:%d]",server_info->m_local_ip,server_info->m_local_port);
                    }
                    else
                    {
                        sh_net::socket_info * info = m_vpn_data_socket.get_clinet_info(new_socket);
                        date_socket_accept(server_info, info);
                        m_p_vpn_context->m_p_log->on_info(stdout, "accept " sh_net_socket_format , sh_net_socket_args(info));
                        
                    }
                }
            }
        }
        m_p_vpn_context->m_p_log->on_info(stdout,"accept epoll thread exit");
        ::close(m_accept_epfd);
    }
