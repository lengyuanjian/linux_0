#pragma once
#include <atomic>


class sh_spinlock 
{
public:
    sh_spinlock() = default;
    sh_spinlock(const sh_spinlock&) = delete;
    sh_spinlock& operator=(const sh_spinlock&) = delete;
    void lock() 
    {
        while (flag.test_and_set(std::memory_order_acquire)) 
        {
            // 可以添加 CPU 暂停指令（如 `__builtin_ia32_pause()` 或 `std::this_thread::yield()`）减少 CPU 占用
        }
    }

    void unlock() 
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;  // 初始化为 false
};