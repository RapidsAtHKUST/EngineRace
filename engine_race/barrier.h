//
// Created by yche on 11/11/18.
//

#pragma once

#include <mutex>
#include <condition_variable>

class Barrier
{
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    std::size_t _count;
public:
    explicit Barrier(std::size_t count) : _count(count) { }
    void Wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (--_count == 0) {
            _cv.notify_all();
        } else {
            _cv.wait(lock, [this] { return _count == 0; });
        }
    }
};