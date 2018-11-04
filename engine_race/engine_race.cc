// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <atomic>
#include <iostream>
#include <chrono>
#include <malloc.h>
#include "log.h"

#include "util.h"
#include "stat.h"

namespace polar_race {
    using namespace std::chrono;

    inline bool file_exists(const char *file_name) {
        struct stat buffer;
        return (stat(file_name, &buffer) == 0);
    }

    using namespace std;

    atomic_int write_num_threads(-1);
//    atomic_int read_num_threads_count (-1);
//
//    // Increase monotonically.
//    atomic_uint time_stamp_(0);
    const string meta_file_name = "polar.meta";
    const string key_file_name = "polar.keys";
    const string value_file_name = "polar.values";

    int64_t polar_str_to_int64(PolarString ps) {
        int64_t int3;
        memcpy(&int3, ps.data(), sizeof(int64_t));
        return int3;
    }

    RetCode Engine::Open(const std::string &name, Engine **eptr) {
        return EngineRace::Open(name, eptr);
    }

    Engine::~Engine() {

    }

/*
 * Complete the functions below to implement you own engine
 */
    EngineRace::EngineRace(const std::string &dir) :
            write_key_file_dp_(nullptr), write_value_file_dp_(nullptr), write_meta_file_dp_(-1),
            write_mmap_meta_file_(nullptr), index_(nullptr), total_cnt_(0) {
        const string meta_file_path = dir + "/" + meta_file_name;
        const string key_file_path = dir + "/" + key_file_name;
        const string value_file_path = dir + "/" + value_file_name;

        write_key_file_dp_ = new int[NUM_THREADS];
        write_value_file_dp_ = new int[NUM_THREADS];

        if (!file_exists(meta_file_path.c_str())) {
            log_info("Initialize the database...");
            const size_t key_file_size = sizeof(KeyEntry) * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;
            const size_t value_file_size = VALUE_SIZE * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;

            write_meta_file_dp_ = open(meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            ftruncate(write_meta_file_dp_, VALUE_SIZE * NUM_THREADS);
            write_mmap_meta_file_ = new char *[NUM_THREADS];

            if (write_meta_file_dp_ < 0) {
                log_info("Fail to create the meta file.");
                exit(-1);
            }

            for (int i = 0; i < NUM_THREADS; ++i) {
                string temp_key = key_file_path + to_string(i);
                string temp_value = value_file_path + to_string(i);

                write_key_file_dp_[i] = open(temp_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                write_value_file_dp_[i] = open(temp_value.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                ftruncate(write_key_file_dp_[i], key_file_size);
                ftruncate(write_value_file_dp_[i], value_file_size);

                if (write_key_file_dp_[i] < 0 || write_value_file_dp_[i] < 0) {
                    log_info("Fail to create key-value files.");
                    exit(-1);
                }

                write_mmap_meta_file_[i] = (char *) mmap(nullptr, VALUE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                                                         write_meta_file_dp_, i * VALUE_SIZE);
                memset(write_mmap_meta_file_[i], 0, sizeof(uint32_t));
            }

            log_info("Create the database successfully.");
        } else {
            log_info("Reload the database.");
            write_meta_file_dp_ = open(meta_file_path.c_str(), O_RDONLY, FILE_PRIVILEGE);
            if (write_meta_file_dp_ < 0) {
                log_info("Fail to open the meta file.");
                exit(-1);
            }

            for (int i = 0; i < NUM_THREADS; ++i) {
                string temp_key = key_file_path + to_string(i);
                string temp_value = value_file_path + to_string(i);

                write_key_file_dp_[i] = open(temp_key.c_str(), O_RDONLY, FILE_PRIVILEGE);
                write_value_file_dp_[i] = open(temp_value.c_str(), O_RDONLY, FILE_PRIVILEGE);

                if (write_key_file_dp_[i] < 0 || write_value_file_dp_[i] < 0) {
                    log_info("Fail to open key-value files.");
                    exit(-1);
                }
            }
            log_info("Open the files.");

            BuildIndex();

            log_info("Reload the database successfully.");
        }
    }

// 1. Open engine
    RetCode EngineRace::Open(const std::string &name, Engine **eptr) {
        if (!file_exists(name.c_str())) {
            int ret = mkdir(name.c_str(), 0755);
            if (ret != 0) {
                log_info("Fail to create the target directory %s.", name.c_str());
                exit(-1);
            }
            log_info("Create the target directory %s.", name.c_str());
        }
        *eptr = new EngineRace(name);

        return kSucc;
    }

    EngineRace::~EngineRace() {
        log_info("Close the database.");

        close(write_meta_file_dp_);
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            close(write_key_file_dp_[i]);
            close(write_value_file_dp_[i]);
            if (write_mmap_meta_file_ != nullptr) {
                munmap(write_mmap_meta_file_[i], VALUE_SIZE);
            }
        }

        delete[] write_key_file_dp_;
        delete[] write_value_file_dp_;
        delete[] write_mmap_meta_file_;
        if (index_ != nullptr) { free(index_); }
        log_info("Close the database successfully.");
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local uint32_t tid = (uint32_t) (++write_num_threads) % NUM_THREADS;
        static thread_local char *mmap_local_meta_file_ = write_mmap_meta_file_[tid];
        static thread_local int local_key_file = write_key_file_dp_[tid];
        static thread_local int local_value_file = write_value_file_dp_[tid];
        static thread_local uint32_t local_block_offset = 0;

        // Write value to the value file.
        pwrite(local_value_file, value.data(), VALUE_SIZE, (uint64_t) local_block_offset * VALUE_SIZE);

        // Write key to the key file.
        KeyEntry key_entry{};
        key_entry.key_ = TO_UINT64(key.data());
        key_entry.value_offset_.partition_ = tid;
        key_entry.value_offset_.block_offset_ = local_block_offset;
        pwrite(local_key_file, &key_entry, sizeof(KeyEntry), local_block_offset * sizeof(KeyEntry));
        local_block_offset += 1;
        // Update the meta data.
        (*(uint32_t *) mmap_local_meta_file_) = local_block_offset;

        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
//        static thread_local int64_t tid = (++read_num_threads_count) % NUM_THREADS;
        static thread_local char value_buffer[VALUE_SIZE];
        uint64_t key_uint = TO_UINT64(key.data());

        KeyEntry tmp{};
        tmp.key_ = key_uint;
        auto it = lower_bound(index_, index_ + total_cnt_, tmp, [](KeyEntry l, KeyEntry r) {
            return l.key_ < r.key_;
        });
        if (it != index_ + total_cnt_ && it->key_ != key_uint)
            return kNotFound;

        ValueOffset value_offset = it->value_offset_;
        pread(write_value_file_dp_[value_offset.partition_], value_buffer, VALUE_SIZE,
              (uint64_t) (value_offset.block_offset_) * VALUE_SIZE);

        value->clear();
        *value = std::string(value_buffer, VALUE_SIZE);
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

    void EngineRace::BuildIndex() {
        // Read meta data.
        uint32_t read_buffer[VALUE_SIZE / sizeof(uint32_t)];
        uint32_t entry_counts[NUM_THREADS];

        for (int i = 0; i < NUM_THREADS; ++i) {
            pread(write_meta_file_dp_, read_buffer, VALUE_SIZE, i * VALUE_SIZE);
            entry_counts[i] = read_buffer[0];
            total_cnt_ += entry_counts[i];
        }
        log_info("total cnt: %d", total_cnt_);
        index_ = (KeyEntry *) (malloc(sizeof(KeyEntry) * total_cnt_));
        int32_t cur_cnt = 0;

        // Read each key file.
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            uint32_t entry_count = entry_counts[i];

            uint32_t passes = entry_count / KEY_READ_BLOCK_COUNT;
            uint32_t remain_entries_count = entry_count - passes * KEY_READ_BLOCK_COUNT;
            size_t read_offset = 0;

            for (uint32_t j = 0; j < passes; ++j) {
                pread(write_key_file_dp_[i], index_ + cur_cnt, KEY_READ_BLOCK_COUNT * sizeof(KeyEntry), read_offset);
//                log_info("%s, cur_cnt: %d", strerror(errno), cur_cnt);
                read_offset += ((size_t) KEY_READ_BLOCK_COUNT) * sizeof(KeyEntry);
                cur_cnt += KEY_READ_BLOCK_COUNT;
            }

            if (remain_entries_count != 0) {
                pread(write_key_file_dp_[i], index_ + cur_cnt, remain_entries_count * sizeof(KeyEntry), read_offset);
//                log_info("%s", strerror(errno));
                cur_cnt += remain_entries_count;
            }
        }
        sort(index_, index_ + total_cnt_, [](KeyEntry l, KeyEntry r) {
            if (l.key_ == r.key_) { return l.value_offset_.block_offset_ > r.value_offset_.block_offset_; }
            return l.key_ < r.key_;
        });
    }
}  // namespace polar_race
