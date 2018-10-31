// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <atomic>
#include <iostream>
#include <chrono>

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
            meta_file_handler_(-1), key_file_handler_(-1), value_file_handler_(-1),
            mmap_meta_file_(nullptr), mmap_key_file_(nullptr), mmap_value_file_(nullptr),
            atomic_value_file_block_offset(-1), global_key_block_count_(0) {

        log_info("Initialize the engine...");

        string meta_file_path = dir + "/" + meta_file_name;
        string key_file_path = dir + "/" + key_file_name;
        string value_file_path = dir + "/" + value_file_name;

        if (!file_exists(key_file_path.c_str())) {
            meta_file_handler_ = open(meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            key_file_handler_ = open(key_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            value_file_handler_ = open(value_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);

            if (key_file_handler_ < 0 || value_file_handler_ < 0 || meta_file_handler_ < 0) {
                log_info("Fail to create key-value files.");
                exit(-1);
            }

            ftruncate(meta_file_handler_, VALUE_SIZE);
            log_info("Truncate the meta file, size: %d bytes.", VALUE_SIZE);

            const size_t key_file_size = sizeof(KeyEntry) * NUM_THREADS * KEY_VALUE_MAX_COUNT_PER_THREAD;
            ftruncate(key_file_handler_, key_file_size);
            log_info("Truncate the key file, size: %zu bytes.", key_file_size);

            const size_t value_file_size = static_cast<size_t> VALUE_SIZE * NUM_THREADS * KEY_VALUE_MAX_COUNT_PER_THREAD;
            ftruncate(value_file_handler_, value_file_size);
            log_info("Truncate the value file, size: %zu bytes.", value_file_size);

            mmap_meta_file_ = (char *) mmap(nullptr, VALUE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, meta_file_handler_, 0);
            mmap_key_file_ = (char *) mmap(nullptr, key_file_size, PROT_READ | PROT_WRITE, MAP_SHARED, key_file_handler_, 0);
            // mmap_value_file_ = (char *) mmap(nullptr, (size_t)VALUE_MMAP_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, value_file_handler_, 0);

            log_info("Create key-value files successfully. The size of the key file is %zu bytes, the size of the value file is %zu bytes", key_file_size, value_file_size);
        }
        else {
            meta_file_handler_ = open(meta_file_path.c_str(), O_RDWR);
            key_file_handler_ = open(key_file_path.c_str(), O_RDWR, FILE_PRIVILEGE);
            value_file_handler_ = open(value_file_path.c_str(), O_RDWR, FILE_PRIVILEGE);

            mmap_meta_file_ = (char *) mmap(nullptr, VALUE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, meta_file_handler_, 0);

            const size_t key_file_size = sizeof(KeyEntry) * NUM_THREADS * KEY_VALUE_MAX_COUNT_PER_THREAD;
            mmap_key_file_ = (char *) mmap(nullptr, key_file_size, PROT_READ | PROT_WRITE, MAP_SHARED, key_file_handler_, 0);

            if (key_file_handler_ < 0 || value_file_handler_ < 0) {
                log_info("Fail to load key-value files.");
                exit(-1);
            }
            log_info("Load key-value files successfully.");

            BuildIndex();
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
        log_info("Close engine...");
        if (mmap_meta_file_ != nullptr) {
            munmap(mmap_meta_file_, VALUE_SIZE);
            log_info("Unmap the meta file successfully.");
        }

        if (mmap_key_file_ != nullptr) {
            const size_t key_file_size = sizeof(KeyEntry) * NUM_THREADS * KEY_VALUE_MAX_COUNT_PER_THREAD;
            munmap(mmap_key_file_, key_file_size);
            log_info("Unmap the key file successfully.");
        }

        if (mmap_value_file_ != nullptr) {
            const size_t value_file_size = static_cast<int64_t> VALUE_SIZE * NUM_THREADS * KEY_VALUE_MAX_COUNT_PER_THREAD;
            munmap(mmap_value_file_, value_file_size);
            log_info("Unmap the value file successfully.");
        }

        close(meta_file_handler_);
        log_info("Close the meta file successfully.");
        close(key_file_handler_);
        log_info("Close the key file successfully.");
        close(value_file_handler_);
        log_info("Close the value file successfully.");

        log_info("Close the engine successfully...");
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        // static thread_local int32_t tid = (++num_threads) % NUM_THREADS;

        // Write value to file.
        int value_file_block_offset = ++atomic_value_file_block_offset;
        size_t value_file_offset = (size_t)value_file_block_offset * VALUE_SIZE;
        pwrite(value_file_handler_, value.data(), VALUE_SIZE, value_file_offset);

        // Write index to file.
        KeyEntry key_entry;
        key_entry.key_ = polar_str_to_int64(key);
        // memcpy(&(key_entry.key_), key.data(), sizeof(int64_t));
        key_entry.value_offset_ = value_file_offset;

        {
            unique_lock<mutex> lock(mtx_update_key_file_);
            int key_file_offset = global_key_block_count_ * sizeof(key_entry);
            memcpy(mmap_key_file_ + key_file_offset, &key_entry, sizeof(key_entry));
            memcpy(mmap_meta_file_, &global_key_block_count_, sizeof(int32_t));
            global_key_block_count_ += 1;
        }

        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        // static thread_local int64_t tid = ++read_num_threads;
        static thread_local char values[VALUE_SIZE];
        int64_t key_int = polar_str_to_int64(key);
        int64_t value_file_offset = index_[key_int];

        pread(value_file_handler_, values, VALUE_SIZE, value_file_offset);

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

    void EngineRace::BuildIndex() {
        index_.reserve(NUM_THREADS * KEY_VALUE_MAX_COUNT_PER_THREAD);
        pread(meta_file_handler_, &global_key_block_count_, sizeof(int32_t), 0);
        global_key_block_count_ += 1;

        KeyEntry key_entries[KEY_READ_BLOCK_COUNT];

        int32_t passes = global_key_block_count_ / KEY_READ_BLOCK_COUNT;
        int32_t remain_entries_count = global_key_block_count_ - passes * KEY_READ_BLOCK_COUNT;
        size_t read_offset = 0;

        int64_t greatest_value_file_offset = 0;

        for (int32_t i = 0; i < passes; ++i) {
            pread(key_file_handler_, (void*)key_entries, KEY_READ_BLOCK_COUNT * sizeof(KeyEntry), read_offset);
            read_offset += KEY_READ_BLOCK_COUNT * sizeof(KeyEntry);

            for (int32_t j = 0; j < KEY_READ_BLOCK_COUNT; ++j) {
                index_[key_entries[j].key_] = key_entries[j].value_offset_;
                if (key_entries[j].value_offset_ > greatest_value_file_offset)
                    greatest_value_file_offset = key_entries[j].value_offset_;
            }
        }

        if (remain_entries_count != 0) {
            pread(key_file_handler_, (void*)key_entries, remain_entries_count * sizeof(KeyEntry), read_offset);
            for (int32_t i = 0; i < remain_entries_count; ++i) {
                index_[key_entries[i].key_] = key_entries[i].value_offset_;
                if (key_entries[i].value_offset_ > greatest_value_file_offset)
                    greatest_value_file_offset = key_entries[i].value_offset_;
            }
        }

        int64_t value_file_block_offset = (greatest_value_file_offset / VALUE_SIZE);
        atomic_value_file_block_offset = (int32_t)value_file_block_offset;
        log_info("Build the index successfully. Size: %d, Cardinality: %zu, Max Value File Block Offset: %lld.", global_key_block_count_, index_.size(), value_file_block_offset);
    }
}  // namespace polar_race
