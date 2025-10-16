#include "sh_socket_manager.h"
#include "sh_socket_define.h"

#include <unistd.h>
namespace sh_net 
{
sh_fd    socket_manager::m_idx=1;

    socket_info * socket_manager::get_clinet_info(int fd)
    {
        socket_info * p_info =  new socket_info();
        m_sh_fd_2_socket_info[m_idx] = p_info;
        m_sh_2_fd[m_idx] = fd;
        p_info->m_fd = fd;
        p_info->m_sh_fd = m_idx++;
        set_socket_addr(p_info);
        return p_info;
    }
    void socket_manager::del_clinet_info(socket_info * p_info)
    { 
        if(m_sh_fd_2_socket_info.find(p_info->m_sh_fd) == m_sh_fd_2_socket_info.end())
        {
            return;
        }
        m_sh_fd_2_socket_info.erase(p_info->m_sh_fd);
        m_sh_2_fd.erase(p_info->m_sh_fd);
        delete p_info;
    }
    void socket_manager::close()
    {
        for(auto & [_, info] : m_sh_fd_2_socket_info)
        {
            ::close(info->m_fd);
            delete info;
        }
    }
}
