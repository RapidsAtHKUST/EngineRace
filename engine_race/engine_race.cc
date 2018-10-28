// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include <atomic>
#include <iostream>

#include "log.h"

namespace polar_race {
    inline bool file_exists(const char *file_name) {
        struct stat buffer;
        return (stat(file_name, &buffer) == 0);
    }

    using namespace std;
    std::atomic_int num_threads(-1);

    const char *value_file_name = "value.redis";
    const char *index_meta_file_name = "index-meta.redis";
    const char *value_meta_file_name = "value-meta.redis";

    int64_t polar_str_to_int64(PolarString ps) {
        int64_t int3;
        memcpy(&int3, ps.data(), sizeof(int64_t));
        return int3;
    }

    RetCode Engine::Open(const std::string &name, Engine **eptr) {
        // recompute value id first
        log_info("%.*s", name.length(), name.c_str());
        return EngineRace::Open(name, eptr);
    }

    Engine::~Engine() {
    }

/*
 * Complete the functions below to implement you own engine
 */
    EngineRace::EngineRace(const std::string &dir) :
            hash_map_mutex_arr_(new mutex[PARTITION_NUM]),
            index_file_fd_arr_(new int[PARTITION_NUM]),
            hash_map_arr_(PARTITION_NUM) {
        // 1st: hash index element count array
        string index_meta_path = dir + "/" + string(index_meta_file_name);
        bool is_first = file_exists(index_meta_path.c_str());
        index_meta_fd_ = open(index_meta_path.c_str(), O_WRONLY | O_CREAT, 0644);
        if (is_first) {
            ftruncate(index_meta_fd_, META_INDEX_SIZE);
            mmap_hash_count_arr_ = (int32_t *) mmap(nullptr, (size_t) META_INDEX_SIZE, \
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, index_meta_fd_, 0);
            memset(mmap_hash_count_arr_, 0, META_INDEX_SIZE);
        } else {
            mmap_hash_count_arr_ = (int32_t *) mmap(nullptr, (size_t) META_INDEX_SIZE, \
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, index_meta_fd_, 0);
        }

        // 2nd: hash table files
        for (int i = 0; i < PARTITION_NUM; i++) {
            string key_file_name = dir + std::string("/index_") + to_string(i) + std::string(".redis");
            is_first = file_exists(key_file_name.c_str());
            index_file_fd_arr_[i] = open(index_meta_path.c_str(), O_WRONLY | O_CREAT, 0644);
        }
        hash_map_mutex_arr_ = new mutex[PARTITION_NUM];

        // 3rd: value meta count array
        string value_meta_path = dir + "/" + string(value_meta_file_name);
        is_first = file_exists(value_meta_path.c_str());
        value_meta_fd_ = open(value_meta_path.c_str(), O_WRONLY | O_CREAT, 0644);
        if (is_first) {
            ftruncate(value_meta_fd_, META_VALUE_SIZE);
            mmap_value_id_pair_arr_ = (pair<int32_t, int32_t> *) mmap(
                    nullptr, (size_t) META_VALUE_SIZE,
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, value_meta_fd_, 0);
            for (int i = 0; i < NUM_THREADS; i++) {
                mmap_value_id_pair_arr_[i].first = i * ID_SKIP;
                mmap_value_id_pair_arr_[i].second = i * ID_SKIP;
            }
        } else {
            mmap_value_id_pair_arr_ = (pair<int32_t, int32_t> *) mmap(
                    nullptr, (size_t) META_VALUE_SIZE,
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, value_meta_fd_, 0);
        }

        // 4th: value file
        string value_file_path = (dir + "/" + value_file_name);
        is_first = file_exists(value_file_path.c_str());
        if (is_first) {
            value_write_only_fd_ = open(value_file_path.c_str(), O_WRONLY | O_CREAT, 0644);
            ftruncate(value_write_only_fd_, static_cast<int64_t >(ID_SKIP) * VALUE_SIZE * NUM_THREADS);
        } else {
            value_write_only_fd_ = open(value_file_path.c_str(), O_WRONLY, 0644);
        }
        value_read_only_fd_ = open(value_file_path.c_str(), O_RDONLY, 0644);
    }

// 1. Open engine
    RetCode EngineRace::Open(const std::string &name, Engine **eptr) {
        *eptr = nullptr;
        auto *engine_race = new EngineRace(name);
        *eptr = engine_race;
        // 4th: value file
        return kSucc;
    }

// 2. Close engine
    EngineRace::~EngineRace() {
        delete[]hash_map_mutex_arr_;
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        // 1st: update the index
        static thread_local int32_t tid = (++num_threads) % NUM_THREADS;
        auto key_int = polar_str_to_int64(key);
        int val_idx = ++value_id;
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);
        {
            unique_lock<mutex> lock(hash_map_mutex_arr_[partition_slot]);
        }
        // 2nd: write the value to the storage
        auto value_off = static_cast<int64_t>(VALUE_SIZE) * val_idx;
        pwrite(value_write_only_fd_, value.data(), VALUE_SIZE, value_off);

        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        auto key_int = polar_str_to_int64(key);
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);
        int64_t offset = node->value_idx_ * static_cast<int64_t >(VALUE_SIZE);
        static thread_local char values[VALUE_SIZE];
        log_info("%lld", offset);
        pread(value_read_only_fd_, values, VALUE_SIZE, offset);

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
