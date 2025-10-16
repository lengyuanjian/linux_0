#include <iostream>
#include "sh_socket_client_once.h"
#include "sh_log.h"
#include <time.h>
using namespace sh_net;

struct tim 
{
    time_t rx_t = 0;
    time_t tx_t = 0;
    int  time_out;
} p_args = {5, 5, 10};

class MySocketEvent : public sh_net::socket_event_once
{
public:
    virtual ~MySocketEvent() {}
    void on_connected(sh_net::socket_info *info) override
    {
        info->m_data.m_ptr = &p_args;
        p_args.rx_t = time(NULL);
        p_args.tx_t = p_args.rx_t;
    }

    void on_disconnected(sh_net::socket_info *info) override
    {
        info->m_data.m_ptr = nullptr;
    }

    void on_a_disconnected(sh_net::socket_info *info) override
    {
        info->m_data.m_ptr = nullptr;
    }

    int on_recv_data(sh_net::socket_info *info) override
    {
        // Handle received data here
        // std::cout<<"len:["<<info->m_data.m_buff_len<<"]\n";
        if(info->m_data.m_ptr)
        {
            tim *p = (tim *)info->m_data.m_ptr;
            p->rx_t = time(NULL);
        }
        return info->m_data.m_buff_len;
    }
    void on_time_out(sh_net::socket_info *info)
    {
        if(info->m_data.m_ptr)
        {
            tim *p = (tim *)info->m_data.m_ptr;
            time_t now = time(NULL);
            time_t dt = now - p->rx_t;
            if(dt >= p->time_out)
            {
                std::cout<<"on_time_out: rx_t["<<p->rx_t<<"] dt["<<dt<<"]\n";
                p->rx_t = now;
                info->m_data.m_buff_len = 0;
                info->m_data.m_ptr = nullptr;
                std::cout<<"timeout close\n";
                sh_net::close_socket(info);

            }
            if((now - p->tx_t) >= (p->time_out/2))
            {
                ::send(info->m_fd, "hello", 5, 0);
                p->tx_t = now;
            }
            
        }
        else
        {
            std::cout<<"on_time_out: info->m_data.m_ptr is null\n";
        }
    }
};
log_event log;
MySocketEvent event;

int main(int argc, char *argv[])
{

    for(int i = 0; i < argc; ++i)
    {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }
    socket_client_once app;
    if(!app.init(&event, &log,"120.46.8.198" ,9526, nullptr,0, 3))
    {
        std::cerr << "Failed to initialize socket client." << std::endl;
        return 1;
    }
    if(!app.start())
    {
        std::cerr << "Failed to start socket client." << std::endl;
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
    app.close();
    return 0;
}
