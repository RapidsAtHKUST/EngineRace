//
// Created by yche on 10/27/18.
//
#include <assert.h>
#include <stdio.h>
#include <string>
#include "include/engine.h"
#include "../engine_race/log.h"

static const char kEnginePath[] = "/tmp/test_engine";
static const char kDumpPath[] = "/tmp/test_dump";

using namespace polar_race;

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

    int64_t round_size = 100000;
    int64_t iter_num = 3;   // switch this to test different settings
    for (int64_t iter = 0; iter < iter_num; iter++) {
        // 1st: write
        Engine::Open(kEnginePath, &engine);

#pragma omp parallel for num_threads(16)
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
        // 2nd: read
        Engine::Open(kEnginePath, &engine);
#pragma omp parallel for num_threads(16) schedule(dynamic, 48)
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
                assert(verify_int == j + i + seed);
                if (is_first) {
                    is_first = false;
                    log_info("%lld, %lld", verify_int, j + i);
                }
            }
            // 2nd: read
        };
    }
    delete engine;

    log_info("correct...");
    return 0;
}

