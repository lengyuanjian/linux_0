#pragma once
#include <atomic>


class sh_rwlock 
{
public:
    sh_rwlock() = default;
    sh_rwlock(const sh_rwlock&) = delete;
    sh_rwlock& operator=(const sh_rwlock&) = delete;
    void lock_read() 
    {
        int32_t expected;
        do 
        {
            expected = m_rwlock.load(std::memory_order_relaxed);
            // 如果当前没有写锁（>=0），则尝试 CAS 增加读计数
            while (expected < 0) 
            {
                // std::this_thread::yield();  // 避免忙等待
                expected = m_rwlock.load(std::memory_order_relaxed);
            }
        } while (!m_rwlock.compare_exchange_weak(expected, expected + 1, std::memory_order_acquire, std::memory_order_relaxed));
    }

    // 释放读锁
    void unlock_read() 
    {
        m_rwlock.fetch_sub(1, std::memory_order_release);
    }

    // 获取写锁（独占）
    void lock_write() 
    {
        int32_t expected = 0;
        // 尝试 CAS 设置写锁（-1），直到成功
        while (!m_rwlock.compare_exchange_weak(expected, -1, std::memory_order_acquire, std::memory_order_relaxed )) 
        {
            expected = 0;  // 重置期望值
            // std::this_thread::yield();  // 避免忙等待
        }
    }

    // 释放写锁
    void unlock_write() 
    {
        m_rwlock.store(0, std::memory_order_release);
    }

private:
    std::atomic_int32_t m_rwlock{0};
};