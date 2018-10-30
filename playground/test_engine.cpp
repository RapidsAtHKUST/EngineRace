//
// Created by yche on 10/27/18.
//
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <cassert>

#include "include/engine.h"
#include "../engine_race/log.h"

static const char kEnginePath[] = "/home/yche/test_engine";
//static const char kEnginePath[] = "/tmp/test_engine";
static const char kDumpPath[] = "/home/yche/test_dump";

using namespace polar_race;

std::string exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

class DumpVisitor : public Visitor {
public:
    DumpVisitor(int *kcnt)
            : key_cnt_(kcnt) {}

    ~DumpVisitor() {}

    void Visit(const PolarString &key, const PolarString &value) {
        printf("Visit %s --> %s\n", key.data(), value.data());
        (*key_cnt_)++;
    }

private:
    int *key_cnt_;
};

int main() {
    int seed = 10;
    Engine *engine = nullptr;
    int NUM_THREADS = 64;
//    int NUM_THREADS = 1;
//    exec("rm -r /home/yche/test_engine/*");
    exec("rm -r /home/yche/test_engine");
//    exec("rm -r /tmp/test_engine/*");

//    int64_t round_size = 2000;
//    int64_t round_size = 255000 * NUM_THREADS;
    int64_t round_size = 25500 * NUM_THREADS;
    int64_t iter_num = 4;   // switch this to test different settings
    for (int64_t iter = 0; iter < iter_num; iter++) {
        // 1st: write
        log_info("iter: %d", iter);
        Engine::Open(kEnginePath, &engine);

#pragma omp parallel for num_threads(NUM_THREADS)
        for (int64_t i = iter * round_size; i < (1 + iter) * round_size; i++) {
            static thread_local char polar_key_str[8];
            static thread_local char polar_value_str[4096];
            memcpy(polar_key_str, &i, sizeof(int64_t));
            for (int64_t j = 0; j < 4096; j += 8) {
                int64_t tmp = j + i + seed;
                memcpy(polar_value_str + j, &tmp, sizeof(int64_t));
            }
            engine->Write(polar_key_str, polar_value_str);
        };
    }

    {
        log_info("read");
        // 2nd: read
        Engine::Open(kEnginePath, &engine);
#pragma omp parallel for num_threads(NUM_THREADS) schedule(dynamic, 48)
        for (int64_t i = 0; i < round_size * iter_num; i++) {
            static thread_local char polar_key_str[8];
            static thread_local std::string tmp_str;
            static thread_local bool is_first = true;
            int64_t verify_int = -1;
            memcpy(polar_key_str, &i, sizeof(int64_t));
            PolarString polar_key(polar_key_str, 8);
            engine->Read(polar_key, &tmp_str);
            for (int64_t j = 0; j < 4096; j += 8) {
                memcpy(&verify_int, tmp_str.c_str() + j, sizeof(int64_t));
                if (verify_int != j + i + seed) {
                    log_info("%d, %d, %d", verify_int, (j + i + seed), i);
                }
                assert(verify_int == j + i + seed);
                if (is_first) {
                    is_first = false;
                    log_info("%lld, %lld", verify_int, j + i + seed);
                }
            }
            // 2nd: read
        };
    }
    delete engine;

    log_info("correct...");
    return 0;
}

