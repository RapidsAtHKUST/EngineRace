// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <atomic>
#include <iostream>

#include "log.h"

#include "util.h"
#include "stat.h"

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
            index_file_fd_arr_(new int[PARTITION_NUM]),
            partition_cardinality_arr_(new int32_t[PARTITION_NUM]),
            hash_map_arr_(PARTITION_NUM),
            partition_mutex_arr_(new mutex[PARTITION_NUM]),
            mmap_index_entry_arr_(new IndexEntry *[PARTITION_NUM]),
            value_id_range_arr_(new ValueMetaEntry[NUM_THREADS]),
            mmap_value_entry_arr_(new char *[NUM_THREADS]) {
        // 1st: init index meta info
        string index_meta_path = dir + "/" + string(index_meta_file_name);
        bool is_first = !file_exists(index_meta_path.c_str());
        index_meta_fd_ = open(index_meta_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
        if (is_first) {
            ftruncate(index_meta_fd_, 0);
            ftruncate(index_meta_fd_, META_INDEX_SIZE);
        }
        mmap_partition_cardinality_arr_ = (int32_t *) mmap(nullptr, (size_t) META_INDEX_SIZE, \
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, index_meta_fd_, 0);
        memcpy(partition_cardinality_arr_, mmap_partition_cardinality_arr_, sizeof(int32_t) * PARTITION_NUM);
        log_info("(1) finish index meta, %s; mem usage: %s KB", strerror(errno),
                 FormatWithCommas(getValue()).c_str());

        // 2nd: index (partitions, k-v mapping)
        int32_t total_cnt = 0;
        for (int i = 0; i < PARTITION_NUM; i++) {
            string index_file_path = dir + std::string("/index-") + to_string(i) + std::string(".redis");
            index_file_fd_arr_[i] = open(index_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            int32_t global_cnt = partition_cardinality_arr_[i];
            log_info("cardinality of %d: %d", i, global_cnt);
            int32_t process_cnt = 0;
            mmap_index_entry_arr_[i] = nullptr;
            // read pairs for index-rebuilding from the files
            for (int j = 0; j < ((global_cnt + INDEX_ENTRY_GROUP_SIZE - 1) / INDEX_ENTRY_GROUP_SIZE); j++) {
                if (mmap_index_entry_arr_[i] != nullptr) {
                    munmap((IndexEntry **) mmap_index_entry_arr_[i], INDEX_CHUNK_MMAP_SIZE);
                }
                mmap_index_entry_arr_[i] = (IndexEntry *)
                        mmap(nullptr, INDEX_CHUNK_MMAP_SIZE, PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_POPULATE, index_file_fd_arr_[i], j * INDEX_CHUNK_MMAP_SIZE);
                for (int k = 0; k < INDEX_ENTRY_GROUP_SIZE && process_cnt < global_cnt; k++, process_cnt++) {
//                    log_info("%lld, %d", mmap_index_entry_arr_[i][k].key_int_, mmap_index_entry_arr_[i][k].val_idx_);
                    hash_map_arr_[i][mmap_index_entry_arr_[i][k].key_int_] =
                            static_cast<int32_t>(mmap_index_entry_arr_[i][k].val_idx_);
                }
            }
            total_cnt += process_cnt;
        }
        log_info("(2) finish index re-building, %s; mem usage: %s KB", strerror(errno),
                 FormatWithCommas(getValue()).c_str());

        // 3rd: value meta info
        string value_meta_path = dir + "/" + string(value_meta_file_name);
        is_first = !file_exists(value_meta_path.c_str());
        value_meta_fd_ = open(value_meta_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
        if (is_first) {
            log_info("first time init value meta");
            ftruncate(value_meta_fd_, META_VALUE_SIZE);
            for (int i = 0; i < NUM_THREADS; i++) {
                value_id_range_arr_[i].beg_idx_ = i * ID_SKIP;
                value_id_range_arr_[i].end_idx_ = i * ID_SKIP;
            }
        }
        mmap_value_id_range_arr_ = (ValueMetaEntry *) mmap(
                nullptr, (size_t) sizeof(ValueMetaEntry) * NUM_THREADS,
                PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, value_meta_fd_, 0);
        if (is_first) {
            memcpy(mmap_value_id_range_arr_, value_id_range_arr_, sizeof(ValueMetaEntry) * NUM_THREADS);
        } else {
            memcpy(value_id_range_arr_, mmap_value_id_range_arr_, sizeof(ValueMetaEntry) * NUM_THREADS);
        }
        log_info("(3) finish value meta, %s; mem usage: %s KB", strerror(errno), FormatWithCommas(getValue()).c_str());

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
            if (value_id_range_arr_[i].end_idx_ > value_id_range_arr_[i].beg_idx_) {
                int64_t chunk_id = (value_id_range_arr_[i].end_idx_ - value_id_range_arr_[i].beg_idx_)
                                   / VALUE_ENTRY_GROUP_SIZE;
                int64_t start_off = value_id_range_arr_[i].beg_idx_ * static_cast<int64_t >(VALUE_SIZE);
                mmap_value_entry_arr_[i] = (char *) mmap(
                        nullptr, (size_t) VALUE_CHUNK_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                        value_write_only_fd_, chunk_id * VALUE_CHUNK_MMAP_SIZE + start_off);
            }
        }
        log_info("(4) finish value, %s; mem usage: %s KB", strerror(errno), FormatWithCommas(getValue()).c_str());
    }

// 1. Open engine
    RetCode EngineRace::Open(const std::string &name, Engine **eptr) {
        *eptr = nullptr;
        if (!file_exists(name.c_str())) {
            int ret = mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            log_info("mkdir name:%s ret: %d", name.c_str(), ret);
        }
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
        munmap(mmap_partition_cardinality_arr_, META_INDEX_SIZE);
        munmap(mmap_value_id_range_arr_, META_VALUE_SIZE);
        delete[]partition_mutex_arr_;
        delete[]index_file_fd_arr_;
        delete[]mmap_index_entry_arr_;
        delete[]mmap_value_entry_arr_;
        delete[]value_id_range_arr_;
        delete[]partition_cardinality_arr_;
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local int32_t tid = (++num_threads) % NUM_THREADS;
        // 1st: write the data file
        int32_t relative_offset = (value_id_range_arr_[tid].end_idx_ -
                                   value_id_range_arr_[tid].beg_idx_) % VALUE_ENTRY_GROUP_SIZE;
        if (relative_offset == 0) {     // move on to the next chunk
            if (mmap_value_entry_arr_[tid] != nullptr) {
                munmap(mmap_value_entry_arr_[tid], VALUE_CHUNK_MMAP_SIZE);
            }
            int64_t offset = static_cast<int64_t>(value_id_range_arr_[tid].end_idx_) * VALUE_SIZE;
            mmap_value_entry_arr_[tid] = (char *) mmap(nullptr, (size_t) VALUE_CHUNK_MMAP_SIZE, PROT_READ | PROT_WRITE,
                                                       MAP_SHARED, value_write_only_fd_, offset);
        }
        memcpy(mmap_value_entry_arr_[tid] + relative_offset * VALUE_SIZE, value.data(), VALUE_SIZE);
        int32_t idx = value_id_range_arr_[tid].end_idx_;
        value_id_range_arr_[tid].end_idx_++;
        memcpy(mmap_value_id_range_arr_ + tid, value_id_range_arr_ + tid, sizeof(ValueMetaEntry));

        // 2nd: write the index file
        auto key_int = polar_str_to_int64(key);
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);
        {
            unique_lock<mutex> lock(partition_mutex_arr_[partition_slot]);
            auto &partition_cardinality_count = partition_cardinality_arr_[partition_slot];
            if (partition_cardinality_count % INDEX_ENTRY_GROUP_SIZE == 0) {
                if (mmap_index_entry_arr_[partition_slot] != nullptr) {
                    munmap(mmap_index_entry_arr_[partition_slot], INDEX_CHUNK_MMAP_SIZE);
                }
                int32_t file_size = partition_cardinality_count * INDEX_ENTRY_SIZE + INDEX_CHUNK_MMAP_SIZE;
                ftruncate(index_file_fd_arr_[partition_slot], file_size);
//                log_info("truncated size: %d", file_size);
                mmap_index_entry_arr_[partition_slot] = (IndexEntry *)
                        mmap(nullptr, INDEX_CHUNK_MMAP_SIZE, PROT_READ | PROT_WRITE,
                             MAP_SHARED, index_file_fd_arr_[partition_slot],
                             partition_cardinality_count * INDEX_ENTRY_SIZE);
            }
            relative_offset = partition_cardinality_count % INDEX_ENTRY_GROUP_SIZE;
            mmap_index_entry_arr_[partition_slot][relative_offset].key_int_ = key_int;
            mmap_index_entry_arr_[partition_slot][relative_offset].val_idx_ = idx;

//            log_info("%d, %d", key_int, idx);
            partition_cardinality_count++;
            memcpy(mmap_partition_cardinality_arr_ + partition_slot, partition_cardinality_arr_ + partition_slot,
                   sizeof(int32_t));
            // update the in-memory data structure
            hash_map_arr_[partition_slot][key_int] = idx;
        }
        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        auto key_int = polar_str_to_int64(key);
        auto partition_slot = static_cast<int32_t>(key_int % PARTITION_NUM);

        assert(hash_map_arr_[partition_slot].contains(key_int));
        int64_t offset = hash_map_arr_[partition_slot][key_int] * static_cast<int64_t>(VALUE_SIZE);
//        log_info("%lld", key_int);

        static thread_local char values[VALUE_SIZE];
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
