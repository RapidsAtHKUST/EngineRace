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

int main() {
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