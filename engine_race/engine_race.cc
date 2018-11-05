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
#include <thread>
#include <algorithm>
#include <random>
#include "log.h"

#include "util.h"
#include "stat.h"

//#include <tbb/parallel_sort.h>
//#include <parallel/algorithm>
#include "parasort.h"

namespace polar_race {
    using namespace std::chrono;

    bool operator<(KeyEntry l, KeyEntry r) {
        if (l.key_ == r.key_) { return l.value_offset_.block_offset_ > r.value_offset_.block_offset_; }
        return l.key_ < r.key_;
    }

    inline bool file_exists(const char *file_name) {
        struct stat buffer;
        return (stat(file_name, &buffer) == 0);
    }

    using namespace std;

    atomic_int write_num_threads(-1);
    atomic_int read_num_threads_count(-1);
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
            write_mmap_meta_file_(nullptr), aligned_buffer_(nullptr), index_(nullptr), total_cnt_(0), dir_(dir) {
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
            aligned_buffer_ = new char *[NUM_THREADS];
            for (int i = 0; i < NUM_THREADS; ++i) {
                string temp_key = key_file_path + to_string(i);
                string temp_value = value_file_path + to_string(i);

                write_key_file_dp_[i] = open(temp_key.c_str(), O_RDONLY, FILE_PRIVILEGE);
                write_value_file_dp_[i] = open(temp_value.c_str(), O_RDONLY | O_DIRECT, FILE_PRIVILEGE);
//                write_value_file_dp_[i] = open(temp_value.c_str(), O_RDONLY, FILE_PRIVILEGE);

                if (write_key_file_dp_[i] < 0 || write_value_file_dp_[i] < 0) {
                    log_info("Fail to open key-value files.");
                    exit(-1);
                }
                aligned_buffer_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, VALUE_SIZE);
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

        bool start_test = false;

        if (total_cnt_ > 30000000 && index_ != nullptr)
            start_test = true;

        close(write_meta_file_dp_);
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            close(write_key_file_dp_[i]);
            close(write_value_file_dp_[i]);
            if (write_mmap_meta_file_ != nullptr) {
                munmap(write_mmap_meta_file_[i], VALUE_SIZE);
            }
            if (aligned_buffer_ != nullptr) {
                free(aligned_buffer_[i]);
            }
        }

        delete[] write_key_file_dp_;
        delete[] write_value_file_dp_;
        delete[] write_mmap_meta_file_;
        delete[] aligned_buffer_;
        if (index_ != nullptr) { free(index_); }

        if (start_test) {
            Benchmark();
        }

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
        static thread_local int64_t tid = (++read_num_threads_count) % NUM_THREADS;
        static thread_local char *value_buffer = aligned_buffer_[tid];;
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

        // Read each key file.
        auto start = high_resolution_clock::now();
        vector<int32_t> prefix_sum(NUM_THREADS + 1, 0);
        for (int tid = 0; tid < NUM_THREADS; tid++) {
            prefix_sum[tid + 1] = prefix_sum[tid] + entry_counts[tid];
        }
        vector<thread> workers(NUM_THREADS);
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            workers[i] = move(thread([&prefix_sum, &entry_counts, i, this]() {
                int32_t cur_cnt = prefix_sum[i];
                uint32_t entry_count = entry_counts[i];

                uint32_t passes = entry_count / KEY_READ_BLOCK_COUNT;
                uint32_t remain_entries_count = entry_count - passes * KEY_READ_BLOCK_COUNT;
                size_t read_offset = 0;

                for (uint32_t j = 0; j < passes; ++j) {
                    pread(write_key_file_dp_[i], index_ + cur_cnt, KEY_READ_BLOCK_COUNT * sizeof(KeyEntry),
                          read_offset);
                    read_offset += ((size_t) KEY_READ_BLOCK_COUNT) * sizeof(KeyEntry);
                    cur_cnt += KEY_READ_BLOCK_COUNT;
                }

                if (remain_entries_count != 0) {
                    pread(write_key_file_dp_[i], index_ + cur_cnt, remain_entries_count * sizeof(KeyEntry),
                          read_offset);
                }
            }));
        }
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            workers[i].join();
        }
        auto end = high_resolution_clock::now();
        log_info("load file, last err: %s; mem usage: %s KB, time: %.3lf s", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);
//        __gnu_parallel::sort(index_, index_ + total_cnt_, [](KeyEntry l, KeyEntry r) {
//            if (l.key_ == r.key_) { return l.value_offset_.block_offset_ > r.value_offset_.block_offset_; }
//            return l.key_ < r.key_;
//        });
        parasort(total_cnt_, index_, 64);
        end = high_resolution_clock::now();

        log_info("total, last err: %s; mem usage: %s KB, time: %.3lf s", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);
    }

    void EngineRace::TestDevice(int open_write_file_flag, uint32_t *write_file_block_offset, uint32_t write_block_num,
                                int open_read_file_flag, uint32_t *read_file_block_offset, uint32_t read_block_num,
                                uint32_t thread_num, uint32_t block_size, uint32_t alignment_size) {
        const string value_file_path = dir_ + "/" + value_file_name;

        write_value_file_dp_ = new int[thread_num];
        aligned_buffer_ = new char*[thread_num];

        for (uint32_t i = 0; i < thread_num; ++i) {
            string temp_value = value_file_path + to_string(i);

            write_value_file_dp_[i] = open(temp_value.c_str(), open_write_file_flag, FILE_PRIVILEGE);

            if (write_value_file_dp_[i] < 0) {
                log_info("Fail to open key-value files.");
                exit(-1);
            }

            aligned_buffer_[i] = (char *) memalign(alignment_size, block_size);
        }

        vector<thread> workers(thread_num);
        auto start = high_resolution_clock::now();

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i] = move(thread([write_file_block_offset, block_size, write_block_num, i, this]() {
                int local_file_dp = write_value_file_dp_[i];
                char* value_buffer = aligned_buffer_[i];

                for (uint32_t j = 0; j < write_block_num; ++j) {
                    size_t write_offset = (size_t)write_file_block_offset[j] * block_size;
                    write(local_file_dp, value_buffer, block_size);
                }
            }));
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i].join();
        }

        auto end = high_resolution_clock::now();
        log_info("Step one %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);

        for (uint32_t i = 0; i < thread_num; ++i) {
            fsync(write_value_file_dp_[i]);
            close(write_value_file_dp_[i]);
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            string temp_value = value_file_path + to_string(i);

            write_value_file_dp_[i] = open(temp_value.c_str(), open_read_file_flag, FILE_PRIVILEGE);

            if (write_value_file_dp_[i] < 0) {
                log_info("Fail to open key-value files.");
                exit(-1);
            }
        }

        start = high_resolution_clock::now();

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i] = move(thread([read_file_block_offset, block_size, read_block_num, i, this]() {
                int local_file_dp = write_value_file_dp_[i];
                char* value_buffer = aligned_buffer_[i];

                for (uint32_t j = 0; j < read_block_num; ++j) {
                    size_t read_offset = (size_t)read_file_block_offset[j] * block_size;
                    pread(local_file_dp, value_buffer, block_size, read_offset);
                }
            }));
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i].join();
        }

        end = high_resolution_clock::now();
        log_info("Step two %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);

        for (uint32_t i = 0; i < thread_num; ++i) {
            fsync(write_value_file_dp_[i]);
            close(write_value_file_dp_[i]);
            free(aligned_buffer_[i]);
        }

        delete[] aligned_buffer_;
        delete[] write_value_file_dp_;
    }

    void EngineRace::Benchmark() {
        const size_t value_file_size = VALUE_SIZE * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;
        vector<uint32_t> block_size_config = {4096, 4096 * 2, 4096 * 4, 4096 * 8, 4096 * 16};
        vector<uint32_t> alignment_size_config = {4096};
        vector<uint32_t> thread_num_config = {64};
        uint32_t flag_config_num = 1;
        vector<int> write_file_flags_config = {O_RDWR | O_APPEND | O_TRUNC | O_DIRECT};
        vector<int> read_file_flags_config = {O_RDONLY | O_DIRECT};

        uint32_t count = 0;
        log_info("Close file..");
        for (uint32_t block_size : block_size_config) {
            uint32_t block_num = (uint32_t)(value_file_size / block_size);
            uint32_t* file_block_offset = new uint32_t[block_num];
            for (uint32_t i = 0; i < block_num; ++i) {
                file_block_offset[i] = i;
            }

            for (uint32_t alignment_size : alignment_size_config) {
                for (uint32_t thread_num : thread_num_config) {
                    for (uint32_t flag_config = 0; flag_config < flag_config_num; ++flag_config) {
                        int write_file_flags = write_file_flags_config[flag_config];
                        int read_file_flags = read_file_flags_config[flag_config];

                        log_info("%d", count++);

                        TestDevice(write_file_flags, file_block_offset, block_num,
                                   read_file_flags, file_block_offset, block_num, thread_num, block_size, alignment_size);
                    }
                }
            }
            delete[] file_block_offset;
        }

//        log_info("Close file end..");
//        count = 0;
//        for (uint32_t block_size : block_size_config) {
//            uint32_t block_num = (uint32_t)(value_file_size / block_size);
//            uint32_t* file_block_offset = new uint32_t[block_num];
//            for (uint32_t i = 0; i < block_num; ++i) {
//                file_block_offset[i] = i;
//            }
//
//            for (uint32_t shuffle_count = 0; shuffle_count < 3; ++shuffle_count) {
//                auto seed = std::chrono::system_clock::now().time_since_epoch().count();
//
//                shuffle(file_block_offset, file_block_offset + block_num, std::default_random_engine(seed));
//            }
//
//            for (uint32_t alignment_size : alignment_size_config) {
//                for (uint32_t thread_num : thread_num_config) {
//                    for (uint32_t flag_config = 0; flag_config < flag_config_num; ++flag_config) {
//                        int write_file_flags = write_file_flags_config[flag_config];
//                        int read_file_flags = read_file_flags_config[flag_config];
//
//                        log_info("%d", count++);
//
//                        TestDevice(write_file_flags, file_block_offset, block_num,
//                                   read_file_flags, file_block_offset, block_num, thread_num, block_size, alignment_size);
//                    }
//                }
//            }
//            delete[] file_block_offset;
//        }
    }
}  // namespace polar_race
