//
// Created by yche on 10/28/18.
//

#include <chrono>


#include "util.h"
#include "stat.h"
#include "log.h"

#include <queue>

using namespace std;
using namespace std::chrono;

#include "../engine_race/blocking_queue_priority.h"

struct ReadTask {
    int32_t tid_;
    int32_t local_offset_;

    ReadTask(int32_t tid, int32_t local_offset) : tid_(tid), local_offset_(local_offset) {}

    bool operator<(const ReadTask &rhs) const {
        return local_offset_ > rhs.local_offset_;
    }
};

void TestOthers() {
    log_info("size: %d", sizeof(pair<int32_t, int32_t>));

    auto start = high_resolution_clock::now();

    queue<int> my_queue;
    my_queue.emplace(1);
    my_queue.emplace(2);

    while (!my_queue.empty()) {
        log_info("%d", my_queue.front());
        my_queue.pop();
    }

    auto end = high_resolution_clock::now();
    log_info("64M tuple insertion cost : %.3lf s, Mem Usage: %s KB",
             duration_cast<milliseconds>(end - start).count() / 1000.0, FormatWithCommas(getValue()).c_str());

}

int main() {
    blocking_priority_queue<ReadTask> read1;
    for (int i = 0; i < 64; i++) {
        read1.push(ReadTask(i, i + 2));
    }
    double time = 0;
    for (int i = 0; i < 64; i++) {
        auto task = read1.pop(time);
        log_info("read: %d, tid: %d", task.local_offset_, task.tid_);
    }
}