// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <atomic>

#include "log.h"
#include "mmap_hash_map.h"

namespace polar_race {
    std::atomic_int num_threads(-1);
    std::atomic_int num_threads_tmp(-1);
    std::atomic_int value_id(-1);

    constexpr int32_t VALUE_SIZE = 4096;
    constexpr int32_t KEY_SIZE = 8;
    constexpr int32_t PARTITION_NUM = 128;
    std::string value_file_name = "value.redis";

    int64_t polar_str_to_int64(PolarString ps) {
        int64_t int3;
        memcpy(&int3, ps.data(), sizeof(int64_t));
        return int3;
    }

    RetCode Engine::Open(const std::string &name, Engine **eptr) {
        static thread_local int32_t tid = ++num_threads_tmp;
        log_info("\"open... tid: %d", tid);
        log_info("%.*s", name.length(), name.c_str());
        return EngineRace::Open(name, eptr);
    }

    Engine::~Engine() {
    }

/*
 * Complete the functions below to implement you own engine
 */

// 1. Open engine
    RetCode EngineRace::Open(const std::string &name, Engine **eptr) {
        *eptr = NULL;
        EngineRace *engine_race = new EngineRace(name);
        engine_race->value_redis_fd_ =
                open((name + "/" + value_file_name).c_str(), O_APPEND | O_WRONLY | O_CREAT, 0644);
        *eptr = engine_race;
        for (int i = 0; i < PARTITION_NUM; i++) {

        }
        return kSucc;
    }

// 2. Close engine
    EngineRace::~EngineRace() {
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local int32_t tid = ++num_threads;
        static thread_local int32_t cnt = 0;
        log_info("write tid: %d", tid);
        cnt++;
        if (cnt == 0) {
            log_info("%lld", polar_str_to_int64(key));
            log_info("%.*s", value.size(), value.data());
        }

        int val_idx = ++value_id;

        // 2nd: write the value to the storage
        auto value_off = static_cast< int64_t>(VALUE_SIZE) * val_idx;
        pwrite(value_redis_fd_, value.data(), VALUE_SIZE, value_off);
        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        return kSucc;
    }

/*
 * NOTICE: Implement 'Range' in quarter-final,
 *         you can skip it in preliminary.
 */
// 5. Applies the given Vistor::Visit function to the result
// of every key-value pair in the key range [first, last),
// in order
// lower=="" is treated as a key before all keys in the database.
// upper=="" is treated as a key after all keys in the database.
// Therefore the following call will traverse the entire database:
//   Range("", "", visitor)
    RetCode EngineRace::Range(const PolarString &lower, const PolarString &upper,
                              Visitor &visitor) {
        return kSucc;
    }

}  // namespace polar_race
