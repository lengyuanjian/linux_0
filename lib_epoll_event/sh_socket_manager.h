#pragma once
#include "sh_socket_define.h"
#include <map>
#include <iterator>

namespace sh_net 
{
    class socket_manager
    {
        static sh_fd                    m_idx;
        std::map<sh_fd, socket_info*>   m_sh_fd_2_socket_info;
        std::map<sh_fd, int>            m_sh_2_fd;
    public:
        using iterator = std::map<sh_fd, socket_info*>::iterator;
        using const_iterator = std::map<sh_fd, socket_info*>::const_iterator;

        iterator begin() { return m_sh_fd_2_socket_info.begin(); }
        iterator end() { return m_sh_fd_2_socket_info.end(); }
        const_iterator begin() const { return m_sh_fd_2_socket_info.begin(); }
        const_iterator end() const { return m_sh_fd_2_socket_info.end(); }

        socket_info * get_clinet_info(int fd);
        void del_clinet_info(socket_info * p_info);
        void close();
    };
}
