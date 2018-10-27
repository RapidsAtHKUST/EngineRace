//
// Created by yche on 10/27/18.
//

#include "polar_string.h"
#include "log.h"

int main() {
    int64_t int1 = -10;
    int64_t int2 = -11;
    using namespace polar_race;
    auto ps = PolarString(reinterpret_cast<char *>(&int1), sizeof(int64_t));
    auto ps2 = PolarString(reinterpret_cast<char *>(&int2), sizeof(int64_t));

    log_info("match status: %d", ps.compare(ps2));
    int64_t int3;
    memcpy(&int3, ps.data(), sizeof(int64_t));
    log_info("%lld", int3);
}