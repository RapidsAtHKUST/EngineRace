//
// Created by yche on 10/27/18.
//

#include "polar_string.h"
#include "log.h"

using namespace polar_race;

int64_t polar_str_to_int64(PolarString ps) {
    int64_t int3;
    memcpy(&int3, ps.data(), sizeof(int64_t));
    return int3;
}

int main() {
    int64_t int1 = -10;
    int64_t int2 = -11;
    auto ps = PolarString(reinterpret_cast<char *>(&int1), sizeof(int64_t));
    auto ps2 = PolarString(reinterpret_cast<char *>(&int2), sizeof(int64_t));

    log_info("match status: %d", ps.compare(ps2));

    log_info("%lld", polar_str_to_int64(ps));
    log_info("%lld", polar_str_to_int64(ps2));
    log_info("%.*s", 8, ps2.data());
}