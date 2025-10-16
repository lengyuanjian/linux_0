#include "../com_lib/net_lib/sh_socket_server.h"
// #include "../../vpn_define/vpv_define.h"
#include <iostream>
#include <signal.h>
#include <vector>
#include <map>
using namespace sh_net;

class server:public sh_net::socket_event
{
    sh_net::socket_server                   m_server;
    log_event *                             m_p_log{nullptr};
    std::map<int, socket_info *>            m_session;       
protected:
    virtual void on_connected(socket_info * info)
    {
        m_session.insert({info->m_fd, info});
    }
    virtual void on_disconnected(socket_info * info)
    {
        auto it = m_session.find(info->m_fd);
        if(it != m_session.end())
        {
            m_session.erase(it);
        }
    }
    virtual void on_a_disconnected(socket_info * info)
    {
        auto it = m_session.find(info->m_fd);
        if(it != m_session.end())
        {
            m_session.erase(it);
        }
    }
    virtual int  on_recv_data(socket_info * info)
    {
        printf("  " sh_net_socket_format "data:%d\n",sh_net_socket_args(info) ,info->m_data.m_buff_len);
        ::send(info->m_fd,info->m_data.m_buff,info->m_data.m_buff_len,0);
        return info->m_data.m_buff_len;
    }


public:
    virtual ~server(){};
    bool init(log_event * p_log, const char * ip, int port);
    bool start();
    void stop();
    void close();

    void exec()
    {
        auto lambda_split = [](const std::string &s)
        {
            std::vector<std::string> ret;
            std::string::size_type start = 0 ,end = 0;
            while (end != std::string::npos) 
            {
                end = s.find_first_of(" ", start);
                ret.push_back(s.substr(start, end - start));
                start = end + 1;
            }
            return ret;
        };
        auto lambda_print = [](const std::vector<std::string> &cmds)
        {
            if(cmds.empty()) {std::cout<<"cmds is empty\n"; return;}
            std::cout << "[" << cmds[0] << "][";
            for (size_t i = 1; i < cmds.size(); i++) 
                std::cout << ((i == 1 )?"":"," )<< cmds[i];
            std::cout << "]" << std::endl;
        };
        std::string input;
        
        while (true) 
        {
            std::cout << "****************************************************\n";
            std::cout << "***\n";
            for(auto &[fd,info]: m_session)
            {
                printf("  " sh_net_socket_format "\n",sh_net_socket_args(info));
            }
            std::cout << "***\n";
            std::cout << "****************************************************\n";
            std::cout << "--Enter 'q' to quit:\n";
            std::getline(std::cin, input);
            if (input == "q") 
            {
                break;
            }
            std::vector<std::string> cmds = lambda_split(input);
            if (cmds.size() == 0) 
            {
                continue;
            }
            lambda_print(cmds);
            if(cmds[0] == "c")
            {

            }
        
        }
    }
};

bool server::init(log_event * p_log,const char * ip, int port)
{
    m_p_log = p_log;
    if(!m_server.init(this, p_log, ip, port))
    {
        m_p_log->on_error(stderr,"init failed");
        return false;
    }
    
    return true;
}
bool server::start()
{
    if(!m_server.start())
    {
        m_p_log->on_error(stderr,"start failed");
        return false;
    }
    return true;
}
void server::stop()
{
    m_server.stop();
}
void server::close()
{
    m_server.close();
}

int main(int argc, char * argv[]) 
{

    signal(SIGPIPE, SIG_IGN);
    if(argc < 3)
    {
        std::cout<<"./exec ip port ip port\n";
        return 1;
    }
    const char * ip = argv[1];
    int port = std::atoi(argv[2]); 
    
    log_event log;
    server                   m_server;
    log_event *m_p_log = &log;


    if(!m_server.init(m_p_log, ip, port))
    {
        m_p_log->on_error(stderr,"init failed");
        return 0;
    }

    if(!m_server.start())
    {
        m_p_log->on_error(stderr,"start failed");
        return 0;
    }
    
    m_server.exec();
    
    m_server.stop();
    
    m_server.close();

    return 0;
}