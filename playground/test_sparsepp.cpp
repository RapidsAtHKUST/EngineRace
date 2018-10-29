//
// Created by yche on 10/28/18.
//

#include <chrono>

#include "../engine_race/sparsepp/spp.h"

#include "util.h"
#include "stat.h"
#include "log.h"

using namespace std;
using namespace std::chrono;

int main() {
    log_info("size: %d", sizeof(pair<int32_t,int32_t>));

    auto start = high_resolution_clock::now();

    // 1.3GB memory consumption with the default setting (single threaded)
    spp::sparse_hash_map<int64_t, int32_t> map;
    for (int64_t i = 0; i < 64000000; i++) {
        map.emplace(i * i, i + 1);
    }
    auto end = high_resolution_clock::now();
    log_info("64M tuple insertion cost : %.3lf s, Mem Usage: %s KB",
             duration_cast<milliseconds>(end - start).count() / 1000.0, FormatWithCommas(getValue()).c_str());

}