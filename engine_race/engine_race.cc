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
#include <cassert>
#include <algorithm>
#include "log.h"

#include "util.h"
#include "stat.h"

//#include <tbb/parallel_sort.h>
//#include <parallel/algorithm>
//#include "parasort.h"

namespace polar_race {
    using namespace std::chrono;
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_end;

    bool operator<(KeyEntry l, KeyEntry r) {
        if (l.key_ == r.key_) { return l.value_offset_.block_offset_ > r.value_offset_.block_offset_; }
        return l.key_ < r.key_;
    }

    inline bool file_exists(const char *file_name) {
        struct stat buffer;
        return (stat(file_name, &buffer) == 0);
    }

    inline uint32_t get_partition_id(uint64_t key) {
        return static_cast<uint32_t>(key % NUM_THREADS);
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
        clock_start = high_resolution_clock::now();
        log_info("sizeof %d, %d", sizeof(off_t), sizeof(off64_t));
        log_info("mem usage: %s KB,  ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        auto ret = EngineRace::Open(name, eptr);
        clock_end = high_resolution_clock::now();
        log_info("After open DB, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        return ret;
    }

    Engine::~Engine() {

    }

/*
 * Complete the functions below to implement you own engine
 */
    EngineRace::EngineRace(const std::string &dir) :
            write_key_file_dp_(nullptr), write_value_file_dp_(nullptr), write_meta_file_dp_(-1),
            write_mmap_meta_file_(nullptr), aligned_buffer_(nullptr) {
        clock_end = high_resolution_clock::now();
        log_info("Start init DB, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
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
            write_mmap_meta_file_ = new uint32_t *[NUM_THREADS];

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

                write_mmap_meta_file_[i] = (uint32_t *) mmap(nullptr, VALUE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                                                             write_meta_file_dp_, i * VALUE_SIZE);
                memset(write_mmap_meta_file_[i], 0, sizeof(uint32_t) * (NUM_THREADS + 1));
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
        clock_end = high_resolution_clock::now();
        log_info("After init DB, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
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
        clock_end = high_resolution_clock::now();
        log_info("Start ~EngineRace(), mem usage: %s KB, time: %.3lf s, ts: %.3lf s",
                 FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
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
        for (KeyEntry *index_partition: index_) {
            free(index_partition);
        }
        clock_end = high_resolution_clock::now();
        log_info("Finish ~EngineRace(), mem usage: %s KB, time: %.3lf s, ts: %.3lf s",
                 FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local uint32_t tid = (uint32_t) (++write_num_threads) % NUM_THREADS;
        static thread_local uint32_t *mmap_local_meta_file_ = write_mmap_meta_file_[tid];
        static thread_local int local_key_file = write_key_file_dp_[tid];
        static thread_local int local_value_file = write_value_file_dp_[tid];
        static thread_local uint32_t local_block_offset = 0;
        if (tid == 0 && local_block_offset == 0) {
            log_info("First Write, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                     duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                     std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() /
                     1000.0);
        }

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
        mmap_local_meta_file_[0] = local_block_offset;
        mmap_local_meta_file_[get_partition_id(key_entry.key_) + 1]++;

        if (local_block_offset % 128 == 0) {
            posix_fadvise(local_value_file, ((size_t) (local_block_offset - 128)) * VALUE_SIZE, 128 * VALUE_SIZE,
                          POSIX_FADV_DONTNEED);
        }
        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        static thread_local int64_t tid = (++read_num_threads_count) % NUM_THREADS;
        static thread_local char *value_buffer = aligned_buffer_[tid];
        static thread_local bool is_first = true;
        if (tid == 0 && is_first) {
            log_info("First Read, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                     duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                     std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() /
                     1000.0);
            is_first = false;
        }

        uint64_t key_uint = TO_UINT64(key.data());

        KeyEntry tmp{};
        tmp.key_ = key_uint;
        auto partition_id = get_partition_id(key_uint);
        auto it = lower_bound(index_[partition_id], index_[partition_id] + total_cnt_[partition_id],
                              tmp, [](KeyEntry l, KeyEntry r) {
                    return l.key_ < r.key_;
                });
        if (it != index_[partition_id] + total_cnt_[partition_id] && it->key_ != key_uint)
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
        log_info("Begin BI, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);

        // Read meta data.
        uint32_t read_buffer[VALUE_SIZE / sizeof(uint32_t)];
        vector<uint32_t> entry_counts(NUM_THREADS);
        vector<vector<uint32_t >> par_prefix_sum_arr(NUM_THREADS + 1, vector<uint32_t>(NUM_THREADS, 0));
        total_cnt_ = vector<uint32_t>(NUM_THREADS, 0);
        for (int tid = 0; tid < NUM_THREADS; ++tid) {
            pread(write_meta_file_dp_, read_buffer, VALUE_SIZE, tid * VALUE_SIZE);
            entry_counts[tid] = read_buffer[0];
            for (int par_id = 0; par_id < NUM_THREADS; ++par_id) {
                par_prefix_sum_arr[tid + 1][par_id] = par_prefix_sum_arr[tid][par_id] + read_buffer[par_id + 1];
//                log_info("prefix (%d, %d): %d", tid, par_id, par_prefix_sum_arr[tid][par_id]);
            }
        }
        index_ = vector<KeyEntry *>(NUM_THREADS, nullptr);

        for (int par_id = 0; par_id < NUM_THREADS; par_id++) {
            total_cnt_[par_id] = par_prefix_sum_arr[NUM_THREADS][par_id];
            index_[par_id] = static_cast<KeyEntry *>(malloc(total_cnt_[par_id] * sizeof(KeyEntry)));
        }

        // Read each key file.
        auto start = high_resolution_clock::now();
        vector<thread> workers(NUM_THREADS);
        for (uint32_t tid = 0; tid < NUM_THREADS; ++tid) {
            workers[tid] = move(thread([&par_prefix_sum_arr, &entry_counts, tid, this]() {
                auto *buffer = (KeyEntry *) malloc(sizeof(KeyEntry) * KEY_READ_BLOCK_COUNT);
                posix_fadvise(write_key_file_dp_[tid], 0, entry_counts[tid] * sizeof(KeyEntry), POSIX_FADV_SEQUENTIAL);
                vector<uint32_t> par_off(NUM_THREADS);
                for (size_t j = 0; j < par_off.size(); j++) {
                    par_off[j] = par_prefix_sum_arr[tid][j];
                }
                uint32_t entry_count = entry_counts[tid];
                uint32_t passes = entry_count / KEY_READ_BLOCK_COUNT;
                uint32_t remain_entries_count = entry_count - passes * KEY_READ_BLOCK_COUNT;
                size_t read_offset = 0;
                for (uint32_t j = 0; j < passes; ++j) {
                    pread(write_key_file_dp_[tid], buffer, KEY_READ_BLOCK_COUNT * sizeof(KeyEntry), read_offset);
                    for (int k = 0; k < KEY_READ_BLOCK_COUNT; k++) {
                        auto par_id = get_partition_id(buffer[k].key_);
                        index_[par_id][par_off[par_id]] = buffer[k];
                        par_off[par_id]++;
                    }
                    read_offset += KEY_READ_BLOCK_COUNT * sizeof(KeyEntry);
                }

                if (remain_entries_count != 0) {
                    pread(write_key_file_dp_[tid], buffer, remain_entries_count * sizeof(KeyEntry), read_offset);
                    for (uint32_t k = 0; k < remain_entries_count; k++) {
                        auto par_id = get_partition_id(buffer[k].key_);
                        index_[par_id][par_off[par_id]] = buffer[k];
                        par_off[par_id]++;
                    }
                }
                free(buffer);
            }));
        }
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            workers[i].join();
        }
        clock_end = high_resolution_clock::now();
        log_info("Build-1, last err: %s; mem usage: %s KB, time: %.3lf s, ts: %.3lf s", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(clock_end - start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);

        uint32_t total_sum = 0;
        for (auto val: total_cnt_) {
            total_sum += val;
        }
        for (uint32_t tid = 0; tid < NUM_THREADS; ++tid) {
            workers[tid] = move(thread([total_sum, tid, this]() {
                sort(index_[tid], index_[tid] + total_cnt_[tid]);

                // check
                if (total_sum > 20000000 && total_cnt_[tid] > 0) {
//                if (total_sum > 2000 && total_cnt_[tid] > 0) {
                    log_info("check in tid: %d..., total cnt: %d", tid, total_sum);
                    auto tmp = index_[tid][0];
                    for (uint32_t i = 1; i < total_cnt_[tid]; i++) {
                        auto &cur_entry = index_[tid][i];
                        if (get_partition_id(cur_entry.key_) != tid || tmp.key_ == cur_entry.key_) {
                            log_error("small - err %zu, %d, %d", tmp.key_, tmp.value_offset_.block_offset_,
                                      tmp.value_offset_.partition_);
                            log_error("large - err %zu, %d, %d", cur_entry.key_,
                                      cur_entry.value_offset_.block_offset_,
                                      cur_entry.value_offset_.partition_);
                        }
                        if (!(tmp < cur_entry)) {
                            log_error("small - err %zu, %d, %d", tmp.key_, tmp.value_offset_.block_offset_,
                                      tmp.value_offset_.partition_);
                            log_error("large - err %zu, %d, %d", cur_entry.key_,
                                      cur_entry.value_offset_.block_offset_,
                                      cur_entry.value_offset_.partition_);
                        }
                        tmp = cur_entry;
                    }
                }
            }));
        }
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            workers[i].join();
        }
        clock_end = high_resolution_clock::now();

        log_info("Build-Total, last err: %s; mem usage: %s KB, time: %.3lf s, ts: %.3lf s", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(clock_end - start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);

        log_info("Finish BI, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
    }
}  // namespace polar_race
