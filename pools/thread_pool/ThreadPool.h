#pragma once
// #include <assert.h>
#include <condition_variable>
#include <functional>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#ifndef CPPHTTPLIB_THREAD_POOL_COUNT
#define CPPHTTPLIB_THREAD_POOL_COUNT                                           \
  ((std::max)(8u, std::thread::hardware_concurrency() > 0                      \
                      ? std::thread::hardware_concurrency() - 1                \
                      : 0))
#endif

class TaskQueue 
{
public:
  TaskQueue() = default;
  virtual ~TaskQueue() = default;

  virtual bool enqueue(std::function<void()> fn) = 0;
  virtual void shutdown() = 0;
};

class ThreadPool final : public TaskQueue 
{
public:
  explicit ThreadPool(size_t n = CPPHTTPLIB_THREAD_POOL_COUNT, size_t mqr = 0)
      : shutdown_(false), max_queued_requests_(mqr) 
    {
        while (n) 
        {
            threads_.emplace_back(worker(*this));
            n--;
        }
    }

  ThreadPool(const ThreadPool &) = delete;
  ~ThreadPool() override = default;

  bool enqueue(std::function<void()> fn) override 
  {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (max_queued_requests_ > 0 && jobs_.size() >= max_queued_requests_) 
      {
        return false;
      }
      jobs_.push_back(std::move(fn));
    }

    cond_.notify_one();
    return true;
  }

  void shutdown() override 
  {
    // Stop all worker threads...
    {
      std::unique_lock<std::mutex> lock(mutex_);
      shutdown_ = true;
    }

    cond_.notify_all();

    // Join...
    for (auto &t : threads_) 
    {
      t.join();
    }
  }

private:
  struct worker 
  {
    explicit worker(ThreadPool &pool) : pool_(pool) {}

    void operator()() 
    {
      for (;;) 
      {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(pool_.mutex_);

          pool_.cond_.wait(lock, [&] { return !pool_.jobs_.empty() || pool_.shutdown_; });

          if (pool_.shutdown_ && pool_.jobs_.empty()) { break; }

          task = pool_.jobs_.front();
          pool_.jobs_.pop_front();
        }
        // assert(true == static_cast<bool>(task));
        try { task();}
        catch (...) {}
      }
    }

    ThreadPool &pool_;
  };
  friend struct worker;

  std::vector<std::thread> threads_;
  std::deque<std::function<void()>> jobs_;

  bool shutdown_;
  size_t max_queued_requests_ = 0;

  std::condition_variable cond_;
  std::mutex mutex_;
};