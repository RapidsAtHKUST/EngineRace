#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "util.h"

// https://github.com/progschj/ThreadPool
class ThreadPool {
public:
    explicit ThreadPool(size_t threads);

    template<class F, class... Args>
    auto enqueue(F &&f, Args &&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;

    ~ThreadPool();

private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads) : stop(false) {
    workers.reserve(threads);
    std::vector<int32_t> affinity_arr_ = {44, 45, 46, 47, 60, 61, 62, 63};
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back(
                [this, i, &affinity_arr_] {
                    setThreadSelfAffinity(affinity_arr_[i]);
                    for (;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

                            if (this->stop && this->tasks.empty()) { return; }

                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        // don't allow enqueueing after stopping the pool
        if (stop) { throw std::runtime_error("enqueue on stopped ThreadPool"); }

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();

    return res;
}

// the destructor joins all threads
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();

    for (auto &worker: workers) { worker.join(); }
}