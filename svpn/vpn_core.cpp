#include <iostream>
#include "vpn_context.h"
#include "s_vpn.h"

bool vpn_core::init(const char * ip, int port)
{
    static vpn_context g_context;
    static vpn_server  g_vpn_server;
    static log_event   g_log;
    
    m_p_context = &g_context;
    m_p_context->m_p_log = &g_log;
    m_p_context->m_p_vpn_server = &g_vpn_server;

    if(!g_vpn_server.init(m_p_context->m_p_log, ip, port))
    {
        return false;
    }
    return true;
}

bool vpn_core::start()
{
    if(!m_p_context->m_p_vpn_server->start())
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
            if (cmds.size() < 8) 
            {
                std::cout << "c out_ip out_port in_ip in_port\n";
                continue;
            } 
            //c 2 172.31.11.200 9424 172.31.11.200 9525 127.0.0.1 3396
            int id = std::atoi(cmds[1].c_str());
            const char *    out_ip      = cmds[2].c_str();
            int             out_port    = std::atoi(cmds[3].c_str());
            const char *    in_ip       = cmds[4].c_str();
            int             in_port     = std::atoi(cmds[5].c_str());
            const char *    local_ip    = cmds[6].c_str();
            int             local_port  = std::atoi(cmds[7].c_str());
            m_p_context->m_p_vpn_server->create_channel_server(id,out_ip,out_port,in_ip,in_port,local_ip,local_port);
        }
        else if(cmds[0] == "d")
        {
            printf("c 2 172.31.11.200 9424 172.31.11.200 9525 127.0.0.1 3396\n");
            m_p_context->m_p_vpn_server->create_channel_server(2 ,"172.31.11.200", 9424, "172.31.11.200", 9525, "127.0.0.1", 3396);
        }
        else if(cmds[0] == "h")
        {
            m_p_context->m_p_vpn_server->printf_client_channel();
        }
        
        
    }
}

void vpn_core::stop()
{
    m_p_context->m_p_vpn_server->stop();
}

void vpn_core::close()
{
    m_p_context->m_p_vpn_server->close();
}
#include <signal.h>
int main(int argc, char * argv[]) 
{
    signal(SIGPIPE, SIG_IGN);
    if(argc < 5)
    {
        std::cout<<"./exec ip port ip port\n";
        return 1;
    }
    const char * ip1 = argv[1];
    unsigned short port1 = std::atoi(argv[2]); 

    const char * ip2 = argv[3];
    unsigned short port2 = std::atoi(argv[4]);
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
