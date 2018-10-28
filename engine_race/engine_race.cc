// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <atomic>

#include "log.h"
#include "mmap_hash_map.h"

namespace polar_race {
    using namespace std;
    std::atomic_int num_threads(-1);
    std::atomic_int num_threads_tmp(-1);
    std::atomic_int value_id(-1);

    constexpr int32_t VALUE_SIZE = 4096;
    constexpr int32_t KEY_SIZE = 8;
    constexpr int32_t PARTITION_NUM = 128;
    const char *value_file_name = "value.redis";

    int64_t polar_str_to_int64(PolarString ps) {
        int64_t int3;
        memcpy(&int3, ps.data(), sizeof(int64_t));
        return int3;
    }

    RetCode Engine::Open(const std::string &name, Engine **eptr) {
        // recompute value id first
        static thread_local int32_t tid = ++num_threads_tmp;
        log_info("\"open... tid: %d", tid);
        cout << "open..." << endl;
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
        *eptr = nullptr;
        auto *engine_race = new EngineRace(name);
        engine_race->value_redis_fd_ =
                open((name + "/" + value_file_name).c_str(), O_RDWR | O_CREAT, 0644);
        cout << "value fd:" << engine_race->value_redis_fd_ << endl;
        engine_race->value_redis_read_only_fd_ =
                open((name + "/" + value_file_name).c_str(), O_RDONLY, 0644);
        *eptr = engine_race;
        engine_race->mmap_hash_map_arr_ = (mmap_hash_map *) malloc(sizeof(mmap_hash_map) * PARTITION_NUM);
        engine_race->mutex_arr_ = new mutex[PARTITION_NUM];
        for (int i = 0; i < PARTITION_NUM; i++) {
            string key_file_name = name + std::string("/redis_index_") + to_string(i);
//            log_info("file name: %.*s", key_file_name.length(), key_file_name.c_str());
            engine_race->mmap_hash_map_arr_[i].open_mmap(key_file_name.c_str());
            value_id += engine_race->mmap_hash_map_arr_[i].get_insert_num();
        }
        log_info("value num: %d", value_id.load());
        return kSucc;
    }

// 2. Close engine
    EngineRace::~EngineRace() {
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local int32_t tid = ++num_threads;
        static thread_local int32_t cnt = 0;
        cnt++;
        if (cnt == 0) {
            log_info("write tid: %d", tid);
            log_info("key: %lld", polar_str_to_int64(key));
            log_info("value: %.*s", value.size(), value.data());
        }
        // 1st: update the index
        auto key_int = polar_str_to_int64(key);
        int val_idx = ++value_id;
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);
        {
            unique_lock<mutex> lock(mutex_arr_[partition_slot]);
            mmap_hash_map_arr_[partition_slot].put(key.data(), KEY_SIZE, val_idx);
        }
        // 2nd: write the value to the storage
        auto value_off = static_cast<int64_t>(VALUE_SIZE) * val_idx;
        pwrite(value_redis_fd_, value.data(), VALUE_SIZE, value_off);

        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        auto key_int = polar_str_to_int64(key);
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);
        node *node = mmap_hash_map_arr_[partition_slot].find(key.data(), KEY_SIZE);
        int64_t offset = node->value_idx_ * static_cast<int64_t >(VALUE_SIZE);
        static thread_local char values[VALUE_SIZE];
        pread(value_redis_read_only_fd_, values, VALUE_SIZE, offset);

        value->clear();
        *value = std::string(values, VALUE_SIZE);
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
