#include <cstddef>
#include <ios>
#include <iostream>
#include "./mem_lib/mem_pool.h"
#include "./err_log/lib_log.h"
int test_client_once();

void test_mem_pool(mem_pool_t *pool)
{
    ;
    int block_size = 4096;
    int block_count = 1024;
    int auto_increment = 256;
    if (create_mem_pool(pool, block_size, block_count, auto_increment) != 0)
    {
        ERROR_LOG("Failed to create memory pool");
        return;
    }
    std::cout << "Memory pool created successfully" << std::endl;   
}
void printf_pool(mem_pool_t *pool)
{
    if(pool == NULL) return;
    std::cout << "Memory pool info:" << std::endl;
    std::cout << "  block size: " << pool->block_size << std::endl;
    std::cout << "  block total: " << pool->block_total << std::endl;
    std::cout << "  free count: " << pool->free_count << std::endl;
    std::cout << "  auto increment: " << pool->auto_increment << std::endl;
    mem_page_t * page = pool->mm;
    std::cout << "  pages:" << std::endl;
    while(page)
    {
        std::cout << "    page: " << static_cast<void*>(page->beg) << " - " << static_cast<void*>(page->end) 
              << " page size: " << (page->end - page->beg) << std::endl;
        page = page->next;
    }
    std::cout << "Memory pool info end" << std::endl;
}

mem_pool_t pool;
int main(int argc, char *argv[])
{
    for(int i = 0; i < argc; ++i)
    {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }
    int main_test(void);
    main_test();
    test_mem_pool(&pool);

    void exec();
    exec();

    return 0;
}

#include <vector>
#include <string>
#include <cstring>
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
            int c = 1024;
            if(cmds.size() >= 2)
            {
                c = std::stoi(cmds[1]);
            }
            char *ptr = NULL;
            for(int i = 0; i < c; ++i)
            {
                ptr = (char *)malloc_mem_pool(&pool);
                memset(ptr, 0, pool.block_size);
            }
            free_mem_pool(&pool,ptr);
        }
        if(cmds[0] == "d")
        {
            printf_pool(&pool);
        }
    
    }
}
