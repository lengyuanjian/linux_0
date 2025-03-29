#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <vector>
#include <set>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sstream>
#include <string>
#include <thread>
#include <map>
namespace sh_server
{
    using sh_fd=unsigned int;
    struct clinet_buff
    {
        int  m_capacity{8*1024};
        int  m_buff_len{0};
        void *m_ptr{nullptr};
        char m_buff[8*1024];
    };
    struct socket_info
    {
        int          m_fd;
        sh_fd        m_sh_fd;
        sockaddr_in  m_address;
        clinet_buff  m_data;
    };
    class event
    {
    public:
        virtual ~event(){}
        virtual void on_debug(const char * msg)
        {std::cout<<msg<<"\n";}
        virtual void on_info(const char * msg) 
        {std::cout<<msg<<"\n";}
        virtual void on_error(const char * msg)
        {std::cout<<msg<<"\n";}
        // virtual void cllocate_clinet_buff(clinet_buff * buff)
        // {
        //     // // if()
        //     // buff->m_buff = new char[4096]();
        //     // buff->m_capacity = 4096;
        //     // buff->m_buff_len = 0;
        // }
        // virtual void release_clinet_buff(clinet_buff * buff)
        // {
        //     // delete [] buff->m_buff;// 
        // }
        virtual void on_connected(socket_info * info) = 0;
        virtual void on_disconnected(socket_info * info) = 0;
        virtual void on_a_disconnected(socket_info * info) = 0;
        virtual int on_recv_data(socket_info * info) = 0;
    };
    
    class net_event
    {
        int             m_server_fd{0};
        event *         m_p_event{nullptr};
        std::string     m_ip;
        int             m_epfd;
        unsigned short  m_port{0};
        std::thread     m_epoll_thread;
        bool            m_epoll_run{true};
        sh_fd           m_idx{1};
        std::map<sh_fd, socket_info*> m_sh_fd_2_socket_info;
        std::map<sh_fd, int>  m_sh_2_fd;
    public:
        bool init(event * p_event, const char *ip, unsigned short port)
        {
            m_p_event = p_event;
            m_ip =ip;
            m_port = port;
            if((m_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
            {
                m_p_event->on_error("socket init failed!");
                return false;
            } 
            int optval = 1;
            if(setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            {
                m_p_event->on_error("socket setsockopt failed!");
                ::close(m_server_fd);
                return false;
            }
            sockaddr_in address;
            address.sin_family = AF_INET;
            inet_pton(AF_INET, ip, &address.sin_addr);
            address.sin_port = htons(port);
            
            if (bind(m_server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
            {
                std::stringstream err;
                err<<"socket bind failed!" << "ip[" << ip << "] port["<<port << "]\n";
                m_p_event->on_error(err.str().c_str());
                return false;
            }
            std::stringstream msg;
            msg<<"server bind" << "ip[" << ip << "] port["<<port << "]\n";
            m_p_event->on_debug(msg.str().c_str());
            return true;
        }
        bool start()
        {
            // 监听连接请求
            if (listen(m_server_fd, 10) < 0) {
                m_p_event->on_error("listen failed");
                return false;
            } 
            m_epoll_thread = std::thread(&net_event::run_epoll, this);   
            return true;        
        }
        void stop()
        {
            m_epoll_run = false;
            m_epoll_thread.join();
        }
        void close()
        {
            for(auto [id, p_info]: m_sh_fd_2_socket_info)
            {
                if(p_info->m_fd != m_server_fd)
                {
                    m_p_event->on_disconnected(p_info);
                }
                epoll_ctl(m_epfd, EPOLL_CTL_DEL,p_info->m_fd,nullptr);
                ::close(p_info->m_fd); 
                //del_clinet_info(info);
                delete p_info;
            } 
        }
        int send_data(int fd, const char * data, int len)
        {
            int ret = ::send(fd, data, len, 0);
            return ret;
        }
        int send_data(sh_fd fd, const char * data, int len)
        {
            auto it = m_sh_2_fd.find(fd);
            if(it != m_sh_2_fd.end())
            {
                return send_data(it->second, data, len);   
            }
            return 0;
        }
        void close(socket_info * p_info)
        {
            epoll_event ev;
            ev.events = EPOLLOUT;
            ev.data.ptr = p_info;
            epoll_ctl(m_epfd, EPOLL_CTL_MOD, p_info->m_fd, &ev);
        }
    protected:
        socket_info * get_clinet_info(int fd)
        {
            socket_info * p_info =  new socket_info();
            m_sh_fd_2_socket_info[m_idx] = p_info;
            m_sh_2_fd[m_idx] = fd;
            p_info->m_fd = fd;
            p_info->m_sh_fd = m_idx++;
            return p_info;
        }
        void del_clinet_info(socket_info * p_info)
        { 
            m_sh_fd_2_socket_info.erase(p_info->m_sh_fd);
            m_sh_2_fd.erase(p_info->m_sh_fd);
            delete p_info;
        }
        void run_epoll()
        {
            m_epfd = epoll_create(1); // 参数只要大于0就可以
            //while(m_epoll_run)
            {
                epoll_event ev;
                ev.events = EPOLLIN;

                ev.data.ptr = get_clinet_info(m_server_fd);

                epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_server_fd, &ev);
                epoll_event evs[1024] = {};
                while(m_epoll_run)
                {
                    int nready = epoll_wait(m_epfd, evs, 1024, 200);
                    for(int i = 0; i < nready; ++i)
                    {
                        socket_info * info = (socket_info *)(evs[i].data.ptr);
                        int sock = info->m_fd;
                        if(sock == m_server_fd)
                        {
                            int new_socket;
                            sockaddr_in address;
                            int addrlen = sizeof(sockaddr_in);
                            if ((new_socket = accept(m_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
                            {
                                m_p_event->on_error("accept error!");
                            }
                            else
                            {
                                socket_info * clinet_info = get_clinet_info(new_socket);
                                clinet_info->m_address = address;
                                
                                epoll_event ev;
                                ev.events = EPOLLIN;
                                ev.data.ptr = clinet_info;
                                epoll_ctl(m_epfd, EPOLL_CTL_ADD,new_socket,&ev);
                                m_p_event->on_connected(clinet_info);
                            }
                            
                        }
                        else if(evs[i].events & EPOLLIN)
                        {
                            auto p_data = &(info->m_data);
                            // m_p_event->cllocate_clinet_buff(p_data);
                            auto len = recv(sock, p_data->m_buff + p_data->m_buff_len, p_data->m_capacity - p_data->m_buff_len, 0);
                            info->m_data.m_buff_len += len;
                            if(len == 0)
                            {
                                m_p_event->on_disconnected(info);
                                ::close(sock);
                                epoll_ctl(m_epfd, EPOLL_CTL_DEL,sock,nullptr);
                                del_clinet_info(info);
                            }
                            else
                            {
                                int use_len = m_p_event->on_recv_data(info);
                                if(use_len < info->m_data.m_buff_len)
                                {
                                    memmove(info->m_data.m_buff, info->m_data.m_buff + use_len, info->m_data.m_buff_len - use_len);
                                    info->m_data.m_buff_len -= use_len;
                                }
                                else
                                {
                                    info->m_data.m_buff_len = 0;
                                }
                            }
                            
                            // m_p_event->release_clinet_buff(p_data);
                        }
                        else if(evs[i].events & EPOLLOUT)
                        {
                            m_p_event->on_a_disconnected(info);
                            ::close(sock);
                            epoll_ctl(m_epfd, EPOLL_CTL_DEL,sock,nullptr);
                            del_clinet_info(info);
                        }   
                    }
                }
            }
            ::close(m_epfd);
        }
    

    private:

    };
};
using namespace sh_server;
class test_event:public sh_server::event
{
    sh_server::net_event    app;
public:
    bool init(const char *ip,int port)
    {
        if(!app.init(this, ip, port))
        {
            std::cout<<"init failed;\n";
            return false;
        }
        if(!app.start())
        {
            std::cout<<"start failed;\n";
            return false;
        }   
        return true;
    }
    void close()
    {
        app.stop();
        app.close();
    }
    virtual ~test_event(){}
    virtual void on_connected(socket_info * info)
    {
         std::cout<<"on_connected:"<< info->m_sh_fd <<":"<< info->m_fd <<"\n";
         ::send(info->m_fd, "123", 3,0);
    }
    virtual void on_disconnected(socket_info * info)
    {
         std::cout<<"on_disconnected:"<< info->m_sh_fd <<":"<< info->m_fd <<"\n";
    }
    virtual void on_a_disconnected(socket_info * info)
    {
         std::cout<<"on_a_disconnected:"<< info->m_sh_fd <<":"<< info->m_fd <<"\n";
    }
    virtual int on_recv_data(socket_info * info)
    {
        int ret = 0;
        std::cout<<"on_data:"<< info->m_sh_fd <<":"<< info->m_fd << " data:"<< info->m_data.m_buff_len <<"\n";
        ret = info->m_data.m_buff_len;
        ::send(info->m_fd, info->m_data.m_buff, info->m_data.m_buff_len,0);
        // app.close(info);
        return ret;
    }
};
int main(int argc, char * argv[]) 
{
    if(argc < 3)
    {
        std::cout<<"./exec ip port\n";
        return 1;
    }

    const char * ip = argv[1];
    unsigned short port = std::atoi(argv[2]); 

    test_event              app_event;
    
    app_event.init(ip,port);
    
    std::string input;
    while (true) {
        std::cout << "Enter 'q' to quit: ";
        std::getline(std::cin, input);
        if (input == "q") {
            break;
        }
    }
    app_event.close();

    
    return 0;
}
