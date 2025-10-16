#include <cstddef>
#include <iostream>
#include "../com_lib/net_lib/sh_log.h"
#include "vpn_core.h"
#include <vector>
#include <string>
#include "business_server.h"
#include "data_channel_server.h"


bool vpn_core::init(const char * ip, int port)
{
    static log_event   g_log;
    static vpn_context g_context;
    g_context.m_p_log = &g_log;
    
    
    m_p_context = &g_context;
    m_p_context->m_p_log = &g_log;
    m_p_context->m_p_business_server = new business_server();
    m_p_context->m_p_data_channel_server = new data_channel_server();

    if(!m_p_context->m_p_business_server->init(m_p_context, ip, port))
    {
        return false;
    }
    if(!m_p_context->m_p_data_channel_server->init(m_p_context))
    {
        return false;
    }
    return true;
}

bool vpn_core::start()
{
    if(!m_p_context->m_p_business_server->start())
    {
        return false;
    }
    if(!m_p_context->m_p_data_channel_server->start())
    {
        return false;
    }
    return true;
}

void vpn_core::exec()
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
        std::cout << "Enter 'q' to quit: ";
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
            if (cmds.size() < 10) 
            {
                std::cout << "c out_ip out_port in_ip in_port\n";
                continue;
            } 
            //c 2 172.31.11.200 9424 172.31.11.200 9525 127.0.0.1 3396
            //c 2 120.16.8.198 17522 120.16.8.198 17523 127.0.0.1 22
            //c 2 172.31.11.200 120.46.8.198 17522 172.31.11.200 120.46.8.198 17523 127.0.0.1 22
            int id = std::atoi(cmds[1].c_str());
            const char *    out_ip      = cmds[2].c_str();
            const char *    out_ip1      = cmds[3].c_str();
            int             out_port    = std::atoi(cmds[4].c_str());
            const char *    in_ip       = cmds[5].c_str();
            const char *    in_ip1       = cmds[6].c_str();
            int             in_port     = std::atoi(cmds[7].c_str());
            const char *    local_ip    = cmds[8].c_str();
            int             local_port  = std::atoi(cmds[9].c_str());
            m_p_context->m_p_business_server->create_channel_server(id, out_ip, out_ip1, out_port, in_ip,in_ip1, in_port, local_ip, local_port);
        }
        else if(cmds[0] == "d" && cmds.size() > 1)
        {
            int id = std::atoi(cmds[1].c_str());
            printf("c %d 172.31.11.200 9424 172.31.11.200 9525 127.0.0.1 3396\n",id);
            const char *    out_ip      = "172.31.11.200";
            const char *    out_ip1      = "120.16.8.198";
            int             out_port    = 9424;
            const char *    in_ip       = "172.31.11.200";
            const char *    in_ip1       = "120.16.8.198";
            int             in_port     = 9525;
            const char *    local_ip    = "127.0.0.1";
            int             local_port  = 3396;
            m_p_context->m_p_business_server->create_channel_server(id, out_ip,out_ip1, out_port, in_ip,in_ip1, in_port, local_ip, local_port);
        
        }
        else if(cmds[0] == "h")
        {
            printf("c out_ip out_port in_ip in_port local_ip local_port\n");
            // m_p_context->m_p_vpn_server->printf_client_channel();
        }
        
        
    }
}

void vpn_core::stop()
{
    m_p_context->m_p_business_server->stop();
    m_p_context->m_p_data_channel_server->stop();
}

void vpn_core::close()
{
    m_p_context->m_p_business_server->close();
    m_p_context->m_p_data_channel_server->close();

    delete m_p_context->m_p_business_server;
    delete m_p_context->m_p_data_channel_server;
}
#include <signal.h>
int main(int argc, char * argv[]) 
{
    signal(SIGPIPE, SIG_IGN);
    if(argc < 3)
    {
        std::cout<<"./exec ip port ip port\n";
        return 1;
    }
    const char * ip1 = argv[1];
    unsigned short port1 = std::atoi(argv[2]); 

    vpn_core app;
    if(!app.init(ip1, port1))
    {
        return 0;
    }
    if(!app.start())
    {
        return 0;
    }

    app.exec();
    
    app.stop();
    
    app.close();
    
    return 0;
}
