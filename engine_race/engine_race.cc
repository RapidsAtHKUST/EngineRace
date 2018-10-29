// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

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
        log_info("\n\n%.*s", name.length(), name.c_str());
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
            hash_map_arr_(PARTITION_NUM),
            mmap_index_entry_arr_(new pair<int64_t, int32_t> *[PARTITION_NUM]),
            mmap_value_entry_arr_(new char *[NUM_THREADS]) {

        // 1st: hash index element count array
//        log_info("%s", strerror(errno));
        string index_meta_path = dir + "/" + string(index_meta_file_name);
        bool is_first = !file_exists(index_meta_path.c_str());

        index_meta_fd_ = open(index_meta_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
//        log_info("%s", strerror(errno));

        if (is_first) {
            ftruncate(index_meta_fd_, META_INDEX_SIZE);
            mmap_hash_meta_count_arr_ = (int32_t *) mmap(nullptr, (size_t) META_INDEX_SIZE, \
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, index_meta_fd_, 0);
            memset(mmap_hash_meta_count_arr_, 0, META_INDEX_SIZE);
        } else {
            mmap_hash_meta_count_arr_ = (int32_t *) mmap(nullptr, (size_t) META_INDEX_SIZE, \
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, index_meta_fd_, 0);
        }
        log_info("finish index meta");
        log_info("%s", strerror(errno));

        // 2nd: hash table files
        int32_t total_cnt = 0;
        for (int i = 0; i < PARTITION_NUM; i++) {
            string key_file_path = dir + std::string("/index-") + to_string(i) + std::string(".redis");
            index_file_fd_arr_[i] = open(key_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            auto global_cnt = mmap_hash_meta_count_arr_[i];
            int32_t process_cnt = 0;
            // read from the files
            mmap_index_entry_arr_[i] = nullptr;
            for (int j = 0; j < ((global_cnt + INDEX_ENTRY_GROUP_SIZE - 1) / INDEX_ENTRY_GROUP_SIZE); j++) {
//                log_info("in-mmap...");
                if (mmap_index_entry_arr_[i] != nullptr) {
                    munmap(mmap_index_entry_arr_[i], INDEX_CHUNK_MMAP_SIZE);
                }
                mmap_index_entry_arr_[i] = (pair<int64_t, int32_t> *)
                        mmap(nullptr, INDEX_CHUNK_MMAP_SIZE, PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_POPULATE, index_file_fd_arr_[i], j * INDEX_CHUNK_MMAP_SIZE);
                for (int k = 0; k < INDEX_ENTRY_GROUP_SIZE && process_cnt < global_cnt; k++, process_cnt++) {
                    hash_map_arr_[i].emplace(mmap_index_entry_arr_[i][k].first, mmap_index_entry_arr_[i][k].second);
                }
            }
            total_cnt += process_cnt;
//            log_info("%d, %d, %d ...", global_cnt, process_cnt, total_cnt);
        }
        log_info("finish index");
        log_info("%s", strerror(errno));

        // 3rd: value meta count array
        string value_meta_path = dir + "/" + string(value_meta_file_name);
        is_first = !file_exists(value_meta_path.c_str());
        value_meta_fd_ = open(value_meta_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
//        log_info("value-meta-fd: %d", value_meta_fd_);
        if (is_first) {
            log_info("first time init value meta");
            ftruncate(value_meta_fd_, META_VALUE_SIZE);
            mmap_value_meta_id_pair_arr_ = (ValueMetaEntry *) mmap(
                    nullptr, (size_t) META_VALUE_SIZE,
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, value_meta_fd_, 0);
            for (int i = 0; i < NUM_THREADS; i++) {
//                log_info("value-meta id: %d", i);
                mmap_value_meta_id_pair_arr_[i].beg_idx = i * ID_SKIP;
                mmap_value_meta_id_pair_arr_[i].end_idx = i * ID_SKIP;
            }
        } else {
            mmap_value_meta_id_pair_arr_ = (ValueMetaEntry *) mmap(
                    nullptr, (size_t) META_VALUE_SIZE,
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, value_meta_fd_, 0);
        }
        log_info("finish value meta");

        // 4th: value file
        string value_file_path = (dir + "/" + value_file_name);
        is_first = !file_exists(value_file_path.c_str());
        if (is_first) {
            value_write_only_fd_ = open(value_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            ftruncate(value_write_only_fd_, static_cast<int64_t >(ID_SKIP) * VALUE_SIZE * NUM_THREADS);
        } else {
            value_write_only_fd_ = open(value_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
        }
        value_read_only_fd_ = open(value_file_path.c_str(), O_RDONLY, FILE_PRIVILEGE);

        for (int i = 0; i < NUM_THREADS; i++) {
            mmap_value_entry_arr_[i] = nullptr;
            if (mmap_value_meta_id_pair_arr_[i].end_idx > mmap_value_meta_id_pair_arr_[i].beg_idx) {
                log_info("%d: %d...", i,
                         mmap_value_meta_id_pair_arr_[i].end_idx - mmap_value_meta_id_pair_arr_[i].beg_idx);
                int64_t chunk_id = (mmap_value_meta_id_pair_arr_[i].end_idx - mmap_value_meta_id_pair_arr_[i].beg_idx)
                                   / VALUE_ENTRY_GROUP_SIZE;
//                log_info("%s", strerror(errno));
                int64_t start_off = mmap_value_meta_id_pair_arr_[i].beg_idx * static_cast<int64_t >(VALUE_SIZE);
                mmap_value_entry_arr_[i] = (char *) mmap(
                        nullptr, (size_t) VALUE_CHUNK_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                        value_write_only_fd_, chunk_id * VALUE_CHUNK_MMAP_SIZE + start_off);
//                log_info("%s", strerror(errno));
            }
        }
        log_info("finish value");
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
        for (int i = 0; i < NUM_THREADS; i++) {
            if (mmap_value_entry_arr_[i] != nullptr) {
                munmap(mmap_value_entry_arr_[i], VALUE_CHUNK_MMAP_SIZE);
            }
        }

        for (int i = 0; i < PARTITION_NUM; i++) {
            if (mmap_index_entry_arr_[i] != nullptr) {
                munmap(mmap_index_entry_arr_[i], INDEX_CHUNK_MMAP_SIZE);
            }
        }
        delete[]hash_map_mutex_arr_;
        delete[]index_file_fd_arr_;
        delete[]mmap_index_entry_arr_;
        delete[]mmap_value_entry_arr_;
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local int32_t tid = (++num_threads) % NUM_THREADS;
        // 1st: write the data file
        int32_t relative_offset = (mmap_value_meta_id_pair_arr_[tid].end_idx -
                                   mmap_value_meta_id_pair_arr_[tid].beg_idx) % VALUE_ENTRY_GROUP_SIZE;
        if (relative_offset == 0) {     // move on to the next chunk
            if (mmap_value_entry_arr_[tid] != nullptr) {
                munmap(mmap_value_entry_arr_[tid], VALUE_CHUNK_MMAP_SIZE);
            }
//            log_info("%s", strerror(errno));
            int64_t offset = static_cast<int64_t >(mmap_value_meta_id_pair_arr_[tid].end_idx) * VALUE_SIZE;
//            log_info("offset:%lld", offset);
            mmap_value_entry_arr_[tid] = (char *) mmap(
                    nullptr, (size_t) VALUE_CHUNK_MMAP_SIZE, PROT_READ | PROT_WRITE,
                    MAP_SHARED, value_write_only_fd_, offset);
//            log_info("%s, %lld, %d, %d ", strerror(errno), offset, mmap_value_meta_id_pair_arr_[tid].end_idx, tid);
        }
        assert(mmap_value_entry_arr_[tid] != nullptr);
        memcpy(mmap_value_entry_arr_[tid] + relative_offset * VALUE_SIZE, value.data(), VALUE_SIZE);
        int32_t idx = mmap_value_meta_id_pair_arr_[tid].end_idx;
        mmap_value_meta_id_pair_arr_[tid].end_idx++;

        // 2nd: write the index file
        auto key_int = polar_str_to_int64(key);
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);
        {
            unique_lock<mutex> lock(hash_map_mutex_arr_[partition_slot]);
            // update the append-only file
            if (mmap_hash_meta_count_arr_[partition_slot] % INDEX_ENTRY_GROUP_SIZE == 0) {
                if (mmap_index_entry_arr_[partition_slot] != nullptr) {
                    munmap(mmap_index_entry_arr_[partition_slot], INDEX_CHUNK_MMAP_SIZE);
                }
//                log_info("%s", strerror(errno));
                ftruncate(index_file_fd_arr_[partition_slot],
                          (mmap_hash_meta_count_arr_[partition_slot] + INDEX_ENTRY_GROUP_SIZE) * INDEX_ENTRY_SIZE);
                mmap_index_entry_arr_[partition_slot] =
                        (pair<int64_t, int32_t> *) mmap(nullptr, INDEX_CHUNK_MMAP_SIZE, PROT_WRITE,
                                                        MAP_SHARED, index_file_fd_arr_[partition_slot],
                                                        mmap_hash_meta_count_arr_[partition_slot] * INDEX_ENTRY_SIZE);
//                log_info("%s", strerror(errno));
            }
            relative_offset = mmap_hash_meta_count_arr_[partition_slot] % INDEX_ENTRY_GROUP_SIZE;
            mmap_index_entry_arr_[partition_slot][relative_offset].first = key_int;
            mmap_index_entry_arr_[partition_slot][relative_offset].second = idx;
            mmap_hash_meta_count_arr_[partition_slot]++;
            // update the in-memory data structure
            hash_map_arr_[partition_slot].emplace(key_int, idx);
        }
        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        auto key_int = polar_str_to_int64(key);
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);

        int64_t offset = hash_map_arr_[partition_slot][key_int] * static_cast<int64_t>(VALUE_SIZE);

        static thread_local char values[VALUE_SIZE];
//        log_info("%lld", offset);
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
