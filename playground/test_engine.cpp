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

#include <byteswap.h>

#include "include/engine.h"
#include "../engine_race/log.h"

#define TO_UINT64(buffer) (*(uint64_t*)(buffer))

//#define ENABLE_READ

static const char kEnginePath[] = "test_engine";

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
    DumpVisitor(int *kcnt, uint64_t key_id, uint64_t seed, PolarString prev)
            : key_id_(key_id), key_str_(new char[sizeof(uint64_t)]), prev_(key_str_, sizeof(uint64_t)), seed_(seed) {
        // Copy
        memcpy(key_str_, prev.data(), sizeof(uint64_t));
    }

    ~DumpVisitor() {
        key_id_ = 0;
    }

    void Visit(const PolarString &key, const PolarString &value) {
        static thread_local bool is_first = true;
//        log_debug("Visit %s --> %s\n", key.data(), value.data());

        // Verify the order
//        assert(prev_.compare(value) <= 0);
        memcpy(key_str_, value.data(), sizeof(uint64_t));
//        log_info("order correct");
        uint64_t verify_int = static_cast<uint64_t>(-1);
        for (uint64_t j = 0; j < 4096; j += 8) {

            // Verify the Correctness.
            key_id_ = TO_UINT64(key.data());
            memcpy(&verify_int, value.data() + j, sizeof(uint64_t));
            if (verify_int != j + key_id_ + seed_) {
                //log_info("Err info: %d, %d, %d", verify_int, (j + key_id_ + seed_), key_id_);
            }
            assert(verify_int == j + key_id_ + seed_);
            if (is_first) {
                is_first = false;
                //log_info("First Range...%lld, %lld", verify_int, j + key_id_ + seed_);
            }
        }
        key_id_++;
    }

private:
    uint64_t key_id_;
    char *key_str_;
    PolarString prev_;
    uint64_t seed_;
};

int main() {
    uint64_t seed = 10;
    uint64_t NUM_THREADS = 64;
    exec("rm -r test_engine");

    uint64_t round_size = 255 * NUM_THREADS;
    uint64_t iter_num = 1;   // switch this to test different settings

    for (uint64_t iter = 0; iter < iter_num; iter++) {
        // 1st: write
        log_info("iter: %d", iter);
        Engine *engine = nullptr;
        Engine::Open(kEnginePath, &engine);

#pragma omp parallel for num_threads(NUM_THREADS)
        for (uint64_t i = iter * round_size; i < (1 + iter) * round_size; i++) {
            static thread_local char polar_key_str[8];
            static thread_local char polar_value_str[4096];
            memcpy(polar_key_str, &i, sizeof(uint64_t));
            for (uint64_t j = 0; j < 4096; j += 8) {
                uint64_t tmp = j + i + seed;
                memcpy(polar_value_str + j, &tmp, sizeof(uint64_t));
            }
            engine->Write(polar_key_str, polar_value_str);
        };
        delete engine;
    }

#ifdef ENABLE_READ
    {
        log_info("read");
        // 2nd: read
        Engine *engine = nullptr;
        Engine::Open(kEnginePath, &engine);
#pragma omp parallel for num_threads(NUM_THREADS) schedule(dynamic, 48)
        for (uint64_t i = 0; i < round_size * iter_num; i++) {
            static thread_local char polar_key_str[8];
            static thread_local std::string tmp_str;
            static thread_local bool is_first = true;
            auto verify_int = static_cast<uint64_t>(-1);
            memcpy(polar_key_str, &i, sizeof(uint64_t));
            PolarString polar_key(polar_key_str, 8);
            engine->Read(polar_key, &tmp_str);
            for (uint64_t j = 0; j < 4096; j += 8) {
                memcpy(&verify_int, tmp_str.c_str() + j, sizeof(uint64_t));
                if (verify_int != j + i + seed) {
                    log_info("%d, %d, %d", verify_int, (j + i + seed), i);
                }
                assert(verify_int == j + i + seed);
                if (is_first) {
                    is_first = false;
                    log_info("%lld, %lld", verify_int, j + i + seed);
                }
            }
        }
        delete engine;
    }
#endif

    {
        log_info("range");
        // 3rd: sequential
        Engine *engine = nullptr;
        Engine::Open(kEnginePath, &engine);
#pragma omp parallel num_threads(NUM_THREADS)
        {
            char tmp_chars_lower[8];
            uint64_t tmp_lower = 0;
            (*(uint64_t *) tmp_chars_lower) = tmp_lower;
            PolarString lower(tmp_chars_lower, 8);

            char tmp_upper_chars[8];
            uint64_t tmp_upper = round_size * iter_num;
            (*(uint64_t *) tmp_upper_chars) = tmp_upper;
            PolarString upper(tmp_upper_chars, 8);

            DumpVisitor visitor(nullptr, 0, seed, lower);
            engine->Range(lower, upper, visitor);
        }
    }
    log_info("correct...");
    return 0;
}

