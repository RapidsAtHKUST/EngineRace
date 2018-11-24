#include <utility>

// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <malloc.h>

#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

#include <byteswap.h>

#include "log.h"
#include "stat.h"

#define STAT

namespace polar_race {
    using namespace std::chrono;
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_end;

    bool operator<(KeyEntry l, KeyEntry r) {
        return l.key_ < r.key_;
    }

    uint32_t branchfree_search(KeyEntry *a, uint32_t n, KeyEntry x) {
        using I = uint32_t;
        const KeyEntry *base = a;
        while (n > 1) {
            I half = n / 2;
            __builtin_prefetch(base + half / 2, 0, 0);
            __builtin_prefetch(base + half + half / 2, 0, 0);
            base = (base[half] < x) ? base + half : base;
            n -= half;
        }
        return (*base < x) + base - a;
    }

    inline bool file_exists(const char *file_name) {
        struct stat buffer;
        return (stat(file_name, &buffer) == 0);
    }

    inline size_t file_size(const char *file_name) {
        struct stat st;
        stat(file_name, &st);
        size_t size = st.st_size;
        return size;
    }

    inline uint32_t get_key_par_id(uint64_t key) {
        return static_cast<uint32_t >((key >> (NUM_THREADS - BUCKET_DIGITS)) & 0xffffffu);
    }

    inline uint64_t polar_str_to_big_endian_uint64(const PolarString &polar_str) {
        static thread_local bool is_first = true;

        if (polar_str.size() == 8) {
            return bswap_64(TO_UINT64(polar_str.data()));
        } else {
            if (is_first) {
                log_info("polar str size: %d", polar_str.size());
                is_first = false;
            }
            char tmp[8];
            memset(tmp, 0, 8);
            memcpy(tmp, polar_str.data(), min<size_t>(8, strlen(polar_str.data())));
            uint64_t tmp_int = bswap_64(TO_UINT64(tmp));
            return polar_str.size() < 8 ? tmp_int : tmp_int + 1;
        }
    }

    using namespace std;

    atomic_int write_num_threads(-1);
    atomic_int read_num_threads_count(-1);
    atomic_int range_num_threads_count(-1);
    atomic_int io_threads_count(-1);

    const string key_file_name = "polar.keys";
    const string value_file_name = "polar.values";

    RetCode Engine::Open(const std::string &name, Engine **eptr) {
        clock_start = high_resolution_clock::now();
        log_info("sizeof %d, %d, %d", sizeof(off_t), sizeof(off64_t), sizeof(KeyEntry));
        log_info("ts: %.3lf s",
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        auto ret = EngineRace::Open(name, eptr);
        clock_end = high_resolution_clock::now();
        log_info("After open DB, time: %.3lf s, ts: %.3lf s",
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
            mmap_meta_cnt_(nullptr), key_file_dp_(nullptr), key_buffer_file_dp_(nullptr),
            mmap_key_aligned_buffer_(nullptr), value_file_dp_(nullptr), value_buffer_file_dp_(nullptr),
            mmap_value_aligned_buffer_(nullptr), partition_mtx_(nullptr), write_barrier_(WRITE_BARRIER_NUM),
            aligned_read_buffer_(nullptr), read_barrier_(READ_BARRIER_NUM),
            is_range_init_(false), range_barrier_ptr_(nullptr), polar_keys_(NUM_THREADS),
            total_time_(0), total_io_sleep_time_(0), wait_get_time_(0),
            val_buffer_max_size_(0), range_io_worker_pool_(nullptr),
            bucket_mutex_arr_(nullptr), bucket_cond_var_arr_(nullptr),
            bucket_is_ready_read_(nullptr), bucket_consumed_num_(nullptr), total_range_num_threads_(0) {
        clock_end = high_resolution_clock::now();
        log_info("Start init DB, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        const string meta_file_path = dir + "/polar.meta";

        const string key_file_path = dir + "/" + key_file_name;
        const string tmp_key_file_path = dir + "/polar.keybuffers";

        const string value_file_path = dir + "/" + value_file_name;
        const string tmp_value_file_path = dir + "/polar.valbuffers";

        key_file_dp_ = new int[BUCKET_NUM];
        key_buffer_file_dp_ = new int[BUCKET_NUM];
        mmap_key_aligned_buffer_ = new uint64_t *[BUCKET_NUM];

        value_file_dp_ = new int[BUCKET_NUM];
        value_buffer_file_dp_ = new int[BUCKET_NUM];
        mmap_value_aligned_buffer_ = new char *[BUCKET_NUM];

        if (!file_exists(meta_file_path.c_str())) {
            log_info("Initialize the database...");
            // Meta.
            meta_cnt_file_dp_ = open(meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            ftruncate(meta_cnt_file_dp_, sizeof(uint32_t) * BUCKET_NUM);
            mmap_meta_cnt_ = (uint32_t *) mmap(nullptr, sizeof(uint32_t) * (BUCKET_NUM),
                                               PROT_READ | PROT_WRITE, MAP_SHARED, meta_cnt_file_dp_, 0);
            memset(mmap_meta_cnt_, 0, sizeof(uint32_t) * (BUCKET_NUM));

            // Value.
            partition_mtx_ = new mutex[BUCKET_NUM];
            for (int i = 0; i < BUCKET_NUM; ++i) {
                string temp_value = value_file_path + to_string(i);
                string temp_buffer_value = tmp_value_file_path + to_string(i);

                value_file_dp_[i] = open(temp_value.c_str(), O_RDWR | O_CREAT | O_DIRECT, FILE_PRIVILEGE);
                if (value_file_dp_[i] < 0) {
                    log_info("fd err of %d: %d, err info: %s", i, value_file_dp_[i], strerror(errno));
                    exit(-1);
                }

                size_t tmp_buffer_value_file_size = VALUE_SIZE * TMP_VALUE_BUFFER_SIZE;
                value_buffer_file_dp_[i] = open(temp_buffer_value.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                if (value_buffer_file_dp_[i] < 0) {
                    log_info("fd err of %d: %d, err info: %s", i, value_buffer_file_dp_[i], strerror(errno));
                    exit(-1);
                }
                ftruncate(value_buffer_file_dp_[i], tmp_buffer_value_file_size);
                mmap_value_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_value_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, value_buffer_file_dp_[i], 0);
            }

            // Key.
            for (int i = 0; i < BUCKET_NUM; ++i) {
                string temp_key = key_file_path + to_string(i);
                string temp_buffer_key = tmp_key_file_path + to_string(i);
                key_file_dp_[i] = open(temp_key.c_str(), O_RDWR | O_CREAT | O_DIRECT, FILE_PRIVILEGE);

                constexpr size_t tmp_buffer_key_file_size = sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE;
                key_buffer_file_dp_[i] = open(temp_buffer_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                ftruncate(key_buffer_file_dp_[i], tmp_buffer_key_file_size);
                mmap_key_aligned_buffer_[i] = (uint64_t *) mmap(nullptr, tmp_buffer_key_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, key_buffer_file_dp_[i], 0);
            }
            log_info("Create the database successfully.");
        } else {
            log_info("Reload the database.");
            meta_cnt_file_dp_ = open(meta_file_path.c_str(), O_RDONLY, FILE_PRIVILEGE);
            mmap_meta_cnt_ = (uint32_t *) mmap(nullptr, sizeof(uint32_t) * (BUCKET_NUM),
                                               PROT_READ, MAP_PRIVATE | MAP_POPULATE, meta_cnt_file_dp_, 0);

            // Value.
            for (int i = 0; i < BUCKET_NUM; ++i) {
                string temp_value = value_file_path + to_string(i);
                value_file_dp_[i] = open(temp_value.c_str(), O_RDONLY | O_DIRECT, FILE_PRIVILEGE);
                value_buffer_file_dp_[i] = -1;
                mmap_value_aligned_buffer_[i] = nullptr;
            }
            // Key.
            for (int i = 0; i < BUCKET_NUM; ++i) {
                string temp_key = key_file_path + to_string(i);
                key_file_dp_[i] = open(temp_key.c_str(), O_RDONLY | O_DIRECT, FILE_PRIVILEGE);
                value_buffer_file_dp_[i] = -1;
                mmap_key_aligned_buffer_[i] = nullptr;
            }
            // Thread.
            aligned_read_buffer_ = new char *[NUM_THREADS];
            for (int i = 0; i < NUM_THREADS; ++i) {
                aligned_read_buffer_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, VALUE_SIZE);
            }
            BuildIndex(dir);
            dir_ = dir;
            log_info("Reload the database successfully.");
        }
        clock_end = high_resolution_clock::now();
        log_info("After init DB, time: %.3lf s, ts: %.3lf s",
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
        log_info("Start ~EngineRace(), time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        // Thread.
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            if (aligned_read_buffer_ != nullptr) {
                free(aligned_read_buffer_[i]);
            }
        }
        delete[] aligned_read_buffer_;

        // Key.
        for (uint32_t i = 0; i < BUCKET_NUM; ++i) {
            if (index_.empty()) {
                if ((mmap_meta_cnt_[i] % TMP_KEY_BUFFER_SIZE) != 0) {
//                    log_info("Flush Key in bucket: %d", i);
                    size_t write_length = TMP_KEY_BUFFER_SIZE * sizeof(uint64_t);
                    size_t write_offset =
                            static_cast<uint64_t>(mmap_meta_cnt_[i] / TMP_KEY_BUFFER_SIZE *
                                                  TMP_KEY_BUFFER_SIZE) * sizeof(uint64_t);
                    pwrite(key_file_dp_[i], mmap_key_aligned_buffer_[i], write_length, write_offset);
                }
                int ret = ftruncate(key_buffer_file_dp_[i], 0);
                if (ret < 0) {
                    log_info("Flush ftruncate Err: %d, %s", ret, strerror(errno));
                }
            }
            if (mmap_key_aligned_buffer_[i] != nullptr) {
                munmap(mmap_key_aligned_buffer_[i], sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE);
            }
            close(key_file_dp_[i]);
            if (key_buffer_file_dp_[i] != -1) {
                close(key_buffer_file_dp_[i]);
            }
        }
        delete[] key_file_dp_;
        delete[] mmap_key_aligned_buffer_;
        // Free indices
        for (KeyEntry *index_partition: index_) {
            free(index_partition);
        }

        // Value.
        for (uint32_t i = 0; i < BUCKET_NUM; ++i) {
            if (index_.empty()) {
                if ((mmap_meta_cnt_[i] % TMP_VALUE_BUFFER_SIZE) != 0) {
                    size_t write_length = (mmap_meta_cnt_[i] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                    size_t write_offset =
                            static_cast<uint64_t>(mmap_meta_cnt_[i] / TMP_VALUE_BUFFER_SIZE *
                                                  TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
//                    log_info("Flush Value in bucket: %d", i);
                    pwrite(value_file_dp_[i], mmap_value_aligned_buffer_[i], write_length, write_offset);
                    int ret = ftruncate(value_buffer_file_dp_[i], 0);
                    if (ret < 0) {
                        log_info("Flush Val ftruncate Err: %d, %s", ret, strerror(errno));
                    }
                }
            }
            if (mmap_value_aligned_buffer_[i] != nullptr) {
                munmap(mmap_value_aligned_buffer_[i], VALUE_SIZE * (size_t) TMP_VALUE_BUFFER_SIZE);
            }
            close(value_file_dp_[i]);
            if (value_buffer_file_dp_[i] != -1) {
                close(value_buffer_file_dp_[i]);
            }
        }
        delete[] value_file_dp_;
        delete[] mmap_value_aligned_buffer_;

        // Meta.
        if (mmap_meta_cnt_ != nullptr) {
            munmap(mmap_meta_cnt_, sizeof(uint32_t) * (BUCKET_NUM));
        }
        close(meta_cnt_file_dp_);

        // Range: Thread.
        if (is_range_init_) {
            for (auto &kv_pair : polar_keys_) {
                if (kv_pair != nullptr)
                    delete[] kv_pair->data();
                delete kv_pair;
            }
        }
        delete range_barrier_ptr_;
        for (char *ptr: value_shared_buffers_) {
            free(ptr);
        }
        delete range_io_worker_pool_;
        if (total_time_ != 0) {
            log_info("Total Range Time: %.9lf s, wait: %.9lf s,  io thread sleep: %.9lf s",
                     total_time_, wait_get_time_, total_io_sleep_time_);
        }
        clock_end = high_resolution_clock::now();
        log_info("Finish ~EngineRace(), time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);

    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local uint32_t tid = (uint32_t) (++write_num_threads) % NUM_THREADS;
        static thread_local uint32_t local_block_offset = 0;
        uint64_t key_int_big_endian = bswap_64(TO_UINT64(key.data()));
        uint64_t key_par_id = get_key_par_id(key_int_big_endian);
#ifdef STAT
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> first_write_clk;
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> last_write_clk;
        if (local_block_offset == 0) {
            first_write_clk = high_resolution_clock::now();
        }
#endif
        if (local_block_offset % 100000 == 0 && local_block_offset < 900000 && tid < WRITE_BARRIER_NUM) {
            write_barrier_.Wait();
        }
        // Value.
        {
            unique_lock<mutex> lock(partition_mtx_[key_par_id]);
            // Write value to the value file, with a tmp file as value_buffer.
            uint32_t val_buffer_offset = (mmap_meta_cnt_[key_par_id] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
            char *value_buffer = mmap_value_aligned_buffer_[key_par_id];
            memcpy(value_buffer + val_buffer_offset, value.data(), VALUE_SIZE);
            if ((mmap_meta_cnt_[key_par_id] + 1) % TMP_VALUE_BUFFER_SIZE == 0) {
                uint64_t write_offset =
                        ((uint64_t) mmap_meta_cnt_[key_par_id] - (TMP_VALUE_BUFFER_SIZE - 1)) * VALUE_SIZE;
                if (write_offset % (4 * 1024 * 1024) == 0) {
                    fallocate(value_file_dp_[key_par_id], 0, write_offset, 4 * 1024 * 1024);
                }
                pwrite(value_file_dp_[key_par_id], value_buffer, VALUE_SIZE * TMP_VALUE_BUFFER_SIZE, write_offset);
            }
            // Write key to the key file.
            uint32_t key_buffer_offset = (mmap_meta_cnt_[key_par_id] % TMP_KEY_BUFFER_SIZE);
            uint64_t *key_buffer = mmap_key_aligned_buffer_[key_par_id];
            key_buffer[key_buffer_offset] = key_int_big_endian;
            if (((mmap_meta_cnt_[key_par_id] + 1) % TMP_KEY_BUFFER_SIZE) == 0) {
                pwrite(key_file_dp_[key_par_id], key_buffer, sizeof(uint64_t) * TMP_KEY_BUFFER_SIZE,
                       ((uint64_t) mmap_meta_cnt_[key_par_id] - (TMP_KEY_BUFFER_SIZE - 1)) * sizeof(uint64_t));
            }
            // Update the meta data.
            mmap_meta_cnt_[key_par_id]++;
        }
        local_block_offset++;
#ifdef STAT
        if (local_block_offset == 1000000) {
            last_write_clk = high_resolution_clock::now();
            log_info("Write Stat of tid %d, elapsed time: %.3lf s, ts: %.3lf s",
                     tid, duration_cast<milliseconds>(last_write_clk - first_write_clk).count() / 1000.0,
                     duration_cast<milliseconds>(last_write_clk.time_since_epoch()).count() / 1000.0);
        }
#endif
        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        static thread_local int64_t tid = (++read_num_threads_count) % NUM_THREADS;
        static thread_local char *value_buffer = aligned_read_buffer_[tid];
        static thread_local bool is_first_not_found = true;
        static thread_local uint32_t local_block_offset = 0;

        uint64_t big_endian_key_uint = bswap_64(TO_UINT64(key.data()));

        KeyEntry tmp{};
        tmp.key_ = big_endian_key_uint;
        auto key_par_id = get_key_par_id(big_endian_key_uint);

        auto it = index_[key_par_id] + branchfree_search(index_[key_par_id], mmap_meta_cnt_[key_par_id], tmp);
        local_block_offset++;

        if (local_block_offset % 100000 == 0 && tid < READ_BARRIER_NUM) {
            read_barrier_.Wait();
        }
        if (it == index_[key_par_id] + mmap_meta_cnt_[key_par_id] || it->key_ != big_endian_key_uint) {
            if (is_first_not_found) {
                log_info("par: %d, key: %zu, %zu", key_par_id, big_endian_key_uint, bswap_64(big_endian_key_uint));
                log_info("not found in tid: %d\n", tid);
                is_first_not_found = false;
            }
            return kNotFound;
        }

        uint64_t val_par_id = get_key_par_id(big_endian_key_uint);
        pread(value_file_dp_[val_par_id], value_buffer, VALUE_SIZE,
              static_cast<uint64_t>(it->value_offset_) * VALUE_SIZE);

        value->assign(value_buffer, VALUE_SIZE);
        return kSucc;
    }

    void EngineRace::ReadBucketToBuffer(uint32_t bucket_id) {
        static thread_local double read_time = 0;
        static thread_local int tid = ++io_threads_count;
        auto range_clock_beg = high_resolution_clock::now();
#ifdef STAT
        if (bucket_id % 64 == 63)
            log_info("In bucket %d, Read in tid: %d, start ts: %.9lf s", bucket_id, tid,
                     std::chrono::duration_cast<std::chrono::nanoseconds>(
                             range_clock_beg.time_since_epoch()).count() /
                     1000000000.0);
#endif
        auto buffer_id = static_cast<uint32_t>(bucket_id % MAX_BUFFER_NUM);

        auto ret = pread(value_file_dp_[bucket_id], value_shared_buffers_[buffer_id],
                         static_cast<uint64_t >(VALUE_SIZE) * mmap_meta_cnt_[bucket_id], 0);
        if (ret < 0) {
            log_info("in range read err: %s, size: %zu", strerror(errno));
        }
        auto range_clock_end = high_resolution_clock::now();
        double elapsed_time = duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                              static_cast<double>(1000000000);

        {
            unique_lock<mutex> lock(total_time_mtx_);
            total_time_ += elapsed_time;
        }
#ifdef STAT
        read_time += elapsed_time;
        if (bucket_id % 64 == 63)
            log_info("In bucket %d, Read time %.9lf s, ts: %.9lf s, tid: %d, acc-time: %.6lf s",
                     bucket_id, elapsed_time,
                     duration_cast<nanoseconds>(range_clock_end.time_since_epoch()).count() /
                     1000000000.0, tid, read_time);
#endif
    }

    void EngineRace::InitForRange(int64_t tid) {
        if (!is_range_init_) {
            unique_lock<mutex> lock(range_mtx_);
            if (tid == 0) {
                range_io_worker_pool_ = new ThreadPool(IO_POOL_SIZE);
                for (int i = 0; i < BUCKET_NUM; i++) {
                    val_buffer_max_size_ = max<uint64_t>(val_buffer_max_size_, mmap_meta_cnt_[i]);
                }
                val_buffer_max_size_ *= VALUE_SIZE;
                log_info("Max Buffer Size: %zu", val_buffer_max_size_);
                value_shared_buffers_ = vector<char *>(MAX_BUFFER_NUM);
                for (uint32_t i = 0; i < MAX_BUFFER_NUM; i++) {
                    value_shared_buffers_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, val_buffer_max_size_);
                }
                const string value_file_path = dir_ + "/" + value_file_name;
                // Value Files.
                for (int i = 0; i < BUCKET_NUM; i++) {
                    string temp_value = value_file_path + to_string(i);
                    close(value_file_dp_[i]);
                    value_file_dp_[i] = open(temp_value.c_str(), O_RDONLY | O_DIRECT, FILE_PRIVILEGE);
                }
                // Sleep here
                auto range_clock_beg = high_resolution_clock::now();
                bucket_mutex_arr_ = new mutex[BUCKET_NUM];
                bucket_cond_var_arr_ = new condition_variable[BUCKET_NUM];
                bucket_is_ready_read_ = new bool[BUCKET_NUM];
                bucket_consumed_num_ = new atomic_int[BUCKET_NUM];
                futures_.resize(BUCKET_NUM);

                usleep(10000);
                auto range_clock_end = high_resolution_clock::now();
                double elapsed_time = duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                                      static_cast<double>(1000000000);
                log_info("Elapsed time in first sync, %.9lf s", 0, elapsed_time);

                total_range_num_threads_ = range_num_threads_count + 1;
                range_barrier_ptr_ = new Barrier(static_cast<size_t>(total_range_num_threads_));
                log_info("Total number of range threads: %zu", total_range_num_threads_);

                is_range_init_ = true;
                range_init_cond_.notify_all();
            } else {
                if (!is_range_init_) {
                    range_init_cond_.wait(lock, [this]() { return is_range_init_; });
                }
            }
        }
        // Submit All IO Jobs
        if (tid == 0) {
            memset(bucket_is_ready_read_, 0, BUCKET_NUM);
            for (uint32_t i = 0; i < MAX_BUFFER_NUM; i++) {
                bucket_is_ready_read_[i] = true;
            }
            for (uint32_t next_bucket_idx = 0; next_bucket_idx < BUCKET_NUM; next_bucket_idx++) {
                bucket_consumed_num_[next_bucket_idx].store(0);
                futures_[next_bucket_idx] = range_io_worker_pool_->enqueue([this, next_bucket_idx]() {
                    unique_lock<mutex> lock(bucket_mutex_arr_[next_bucket_idx]);
                    if (!bucket_is_ready_read_[next_bucket_idx]) {
                        auto range_clock_beg = high_resolution_clock::now();

                        bucket_cond_var_arr_[next_bucket_idx].wait(lock, [this, next_bucket_idx]() {
                            return bucket_is_ready_read_[next_bucket_idx];
                        });
                        auto range_clock_end = high_resolution_clock::now();
                        double sleep_time = duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                                            static_cast<double>(1000000000);
//                        log_info("IO sleep in bucket :%d，elapsed time: %.9lf", next_bucket_idx, sleep_time);

                        {
                            unique_lock<mutex> lock_time(total_time_mtx_);
                            total_io_sleep_time_ += sleep_time;
                        }
                    }
                    ReadBucketToBuffer(next_bucket_idx);
                    return;
                });
            }
        }
        range_barrier_ptr_->Wait();
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
        static thread_local int64_t tid = (++range_num_threads_count) % NUM_THREADS;
        static thread_local uint32_t local_block_offset = 0;
        static thread_local uint32_t invocation_num = 0;
        static thread_local PolarString *polar_key_ptr_;
        static thread_local PolarString polar_val_ptr_;

        auto range_init_start_clock = high_resolution_clock::now();

        if (tid == 0) {
            log_info("Start range ts: %.9lf s in tid %d", duration_cast<nanoseconds>(
                    range_init_start_clock.time_since_epoch()).count() / 1000000000.0, tid);
        }
        // Thread Local Key/Value Init.
        if (local_block_offset == 0) {
            char *key_chars = new char[sizeof(uint64_t)];
            polar_keys_[tid] = new PolarString(key_chars, sizeof(uint64_t));
            polar_key_ptr_ = polar_keys_[tid];
        }
#ifdef RANGE_STAT
        uint64_t key_lower_uint64 = polar_str_to_big_endian_uint64(lower);
        uint64_t key_upper_uint64 = polar_str_to_big_endian_uint64(upper);
        if (invocation_num < 10) {
            invocation_num++;
            log_info("tid: %d, [%zu, %zu), sizes: %zu, %zu, invocation times: %d", tid, key_lower_uint64,
                     key_upper_uint64, lower.size(), upper.size(), invocation_num);
        }
#endif
        InitForRange(tid);

        auto range_init_end_clock = high_resolution_clock::now();
        double elapsed_time = duration_cast<nanoseconds>(range_init_end_clock - range_init_start_clock).count() /
                              static_cast<double>(1000000000);
        if (tid == 0) {
            log_info("Init elapsed time in tid %d, %.9lf s, ts: %.9lf s", tid, elapsed_time,
                     std::chrono::duration_cast<std::chrono::nanoseconds>(
                             range_init_end_clock.time_since_epoch()).count() / 1000000000.0);
        }
        // 2-level Loop.
        uint32_t lower_key_par_id = 0;
        uint32_t upper_key_par_id = BUCKET_NUM - 1;
        for (uint32_t key_par_id = lower_key_par_id; key_par_id < upper_key_par_id + 1; key_par_id++) {
            if (tid == 0) {
                auto wait_start_clock = high_resolution_clock::now();
                futures_[key_par_id].get();
                auto wait_end_clock = high_resolution_clock::now();
                elapsed_time = duration_cast<nanoseconds>(wait_end_clock - wait_start_clock).count() /
                               static_cast<double>(1000000000);
                wait_get_time_ += elapsed_time;
            } else {
                futures_[key_par_id].get();
            }

            uint32_t in_par_id_beg = 0;
            uint32_t in_par_id_end = mmap_meta_cnt_[key_par_id];
            uint64_t prev_key = 0;
            uint32_t duplicates_num = 0;
            uint32_t inner_loop_buffer_idx = key_par_id % MAX_BUFFER_NUM;
            for (uint32_t in_par_id = in_par_id_beg; in_par_id < in_par_id_end; in_par_id++) {
                // Skip the equalities.
                uint64_t big_endian_key = index_[key_par_id][in_par_id].key_;
                if (in_par_id != in_par_id_beg) {
                    if (big_endian_key == prev_key) {
                        if (tid == 0 && duplicates_num < 3 && key_par_id % 256 == 0) {
                            log_info("duplicates...%d", duplicates_num);
                            duplicates_num++;
                        }
                        continue;
                    }
                }
                prev_key = big_endian_key;

                // Key (to little endian first).
                (*(uint64_t *) polar_key_ptr_->data()) = bswap_64(big_endian_key);

                // Value.
                uint64_t val_id = index_[key_par_id][in_par_id].value_offset_;
                polar_val_ptr_ = PolarString(value_shared_buffers_[inner_loop_buffer_idx] + val_id * VALUE_SIZE,
                                             VALUE_SIZE);
                // Visit Key/Value.
                visitor.Visit(*polar_key_ptr_, polar_val_ptr_);
            }
            // End of inner loop, Submit IO Jobs.
            int32_t my_order = ++bucket_consumed_num_[key_par_id];
            if (my_order == total_range_num_threads_) {
                uint32_t next_bucket_idx = key_par_id + MAX_BUFFER_NUM;

                // Notify
                if (next_bucket_idx < upper_key_par_id + 1) {
                    unique_lock<mutex> lock(bucket_mutex_arr_[next_bucket_idx]);
                    bucket_is_ready_read_[next_bucket_idx] = true;
                    bucket_cond_var_arr_[next_bucket_idx].notify_all();
                }
            }
        }
        if (tid == 0) { log_info("one round ok..."); }
        range_barrier_ptr_->Wait();
        return kSucc;
    }

    void EngineRace::FlushTmpFiles(string dir) {
        vector<thread> workers(NUM_FLUSH_TMP_THREADS);
        for (uint32_t tid = 0; tid < NUM_FLUSH_TMP_THREADS; tid++) {
            workers[tid] = thread([this, tid, dir]() {
                const string value_file_path = dir + "/" + value_file_name;
                const string tmp_value_file_path = dir + "/polar.valbuffers";
                const string key_file_path = dir + "/" + key_file_name;
                const string tmp_key_file_path = dir + "/polar.keybuffers";

                for (int bucket_id = tid; bucket_id < BUCKET_NUM; bucket_id += NUM_FLUSH_TMP_THREADS) {
                    // Flush Values.
                    string temp_value = value_file_path + to_string(bucket_id);
                    string temp_buffer_value = tmp_value_file_path + to_string(bucket_id);
                    if (file_size(temp_buffer_value.c_str()) != 0) {
                        if ((mmap_meta_cnt_[bucket_id] % TMP_VALUE_BUFFER_SIZE) != 0) {
                            value_buffer_file_dp_[bucket_id] = open(temp_buffer_value.c_str(), O_RDONLY,
                                                                    FILE_PRIVILEGE);
                            size_t tmp_buffer_value_file_size = VALUE_SIZE * TMP_VALUE_BUFFER_SIZE;
                            mmap_value_aligned_buffer_[bucket_id] = (char *) mmap(
                                    nullptr, tmp_buffer_value_file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE,
                                    value_buffer_file_dp_[bucket_id], 0);

                            size_t write_length = (mmap_meta_cnt_[bucket_id] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                            size_t write_offset =
                                    static_cast<uint64_t>(mmap_meta_cnt_[bucket_id] / TMP_VALUE_BUFFER_SIZE *
                                                          TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                            auto tmp_fd = open(temp_value.c_str(), O_RDWR, FILE_PRIVILEGE);
                            log_info("Flush Val in bucket: %d", bucket_id);
                            pwrite(tmp_fd, mmap_value_aligned_buffer_[bucket_id], write_length, write_offset);
                            close(tmp_fd);
                        }
                        ftruncate(value_buffer_file_dp_[bucket_id], 0);
                    }

                    // Flush Keys.
                    string temp_buffer_key = tmp_key_file_path + to_string(bucket_id);
                    if (file_size(temp_buffer_key.c_str()) != 0) {
                        if ((mmap_meta_cnt_[bucket_id] % TMP_KEY_BUFFER_SIZE) != 0) {
                            key_buffer_file_dp_[bucket_id] = open(temp_buffer_key.c_str(), O_RDONLY, FILE_PRIVILEGE);
                            constexpr size_t tmp_buffer_key_file_size = sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE;
                            mmap_key_aligned_buffer_[bucket_id] = (uint64_t *) mmap(
                                    nullptr, tmp_buffer_key_file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE,
                                    key_buffer_file_dp_[bucket_id], 0);

                            string temp_key = key_file_path + to_string(bucket_id);
                            log_info("Flush Key in bucket: %d", bucket_id);
                            size_t write_length = (mmap_meta_cnt_[bucket_id] % TMP_KEY_BUFFER_SIZE) * sizeof(uint64_t);
                            size_t write_offset =
                                    static_cast<uint64_t>(mmap_meta_cnt_[bucket_id] / TMP_KEY_BUFFER_SIZE *
                                                          TMP_KEY_BUFFER_SIZE) * sizeof(uint64_t);
                            auto tmp_fd = open(temp_key.c_str(), O_RDWR, FILE_PRIVILEGE);
                            pwrite(tmp_fd, mmap_key_aligned_buffer_[bucket_id], write_length, write_offset);
                            close(tmp_fd);
                        }
                        ftruncate(key_buffer_file_dp_[bucket_id], 0);
                    }
                }
            });
        }
        for (uint32_t i = 0; i < NUM_FLUSH_TMP_THREADS; i++) {
            workers[i].join();
        }
    }

    void EngineRace::BuildIndex(string dir) {
        clock_end = high_resolution_clock::now();
        log_info("Begin BI, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        // Key Cnt, Index Allocation.
        index_ = vector<KeyEntry *>(BUCKET_NUM, nullptr);
        for (int key_par_id = 0; key_par_id < BUCKET_NUM; key_par_id++) {
            index_[key_par_id] = static_cast<KeyEntry *>(malloc(mmap_meta_cnt_[key_par_id] * sizeof(KeyEntry)));
        }

        // Flush tmp Files
        FlushTmpFiles(std::move(dir));
        clock_end = high_resolution_clock::now();
        log_info("After Flush Files, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);

        // Read each key file.
        auto **local_buffers_g = new uint64_t *[NUM_READ_KEY_THREADS];
        for (uint32_t tid = 0; tid < NUM_READ_KEY_THREADS; ++tid) {
            local_buffers_g[tid] = (uint64_t *) memalign(FILESYSTEM_BLOCK_SIZE,
                                                         sizeof(uint64_t) * KEY_READ_BLOCK_COUNT);
        }
        vector<thread> workers(NUM_READ_KEY_THREADS);
        for (uint32_t tid = 0; tid < NUM_READ_KEY_THREADS; ++tid) {
            workers[tid] = move(thread([tid, local_buffers_g, this]() {
                uint64_t *local_buffer = local_buffers_g[tid];
                for (uint32_t key_par_id = tid; key_par_id < BUCKET_NUM; key_par_id += NUM_READ_KEY_THREADS) {
                    uint32_t entry_count = mmap_meta_cnt_[key_par_id];
                    if (entry_count > 0) {
#ifdef STAT_ENTRY_CNT
                        log_info("entry count: %d", entry_count);
#endif
                        uint32_t passes = entry_count / KEY_READ_BLOCK_COUNT;
                        uint32_t remain_entries_count = entry_count - passes * KEY_READ_BLOCK_COUNT;
                        uint32_t file_offset = 0;
                        size_t read_offset = 0;

                        for (uint32_t j = 0; j < passes; ++j) {
                            auto ret = pread(key_file_dp_[key_par_id], local_buffer,
                                             KEY_READ_BLOCK_COUNT * sizeof(uint64_t), read_offset);
                            if (ret != KEY_READ_BLOCK_COUNT * sizeof(uint64_t)) {
                                log_info("ret: %d, err: %s", ret, strerror(errno));
                            }
                            for (uint32_t k = 0; k < KEY_READ_BLOCK_COUNT; k++) {
                                index_[key_par_id][file_offset].key_ = local_buffer[k];
                                index_[key_par_id][file_offset].value_offset_ = file_offset;
                                file_offset++;
                            }
                            read_offset += KEY_READ_BLOCK_COUNT * sizeof(uint64_t);
                        }

                        if (remain_entries_count != 0) {
                            size_t num_bytes = (remain_entries_count * sizeof(uint64_t) + FILESYSTEM_BLOCK_SIZE - 1) /
                                               FILESYSTEM_BLOCK_SIZE * FILESYSTEM_BLOCK_SIZE;
                            auto ret = pread(key_file_dp_[key_par_id], local_buffer,
                                             num_bytes, read_offset);
                            if (ret < remain_entries_count * sizeof(uint64_t)) {
                                log_info("ret: %d, err: %s", ret, strerror(errno));
                            }
                            for (uint32_t k = 0; k < remain_entries_count; k++) {
                                index_[key_par_id][file_offset].key_ = local_buffer[k];
                                index_[key_par_id][file_offset].value_offset_ = file_offset;
                                file_offset++;
                            }
                        }
                        sort(index_[key_par_id], index_[key_par_id] + entry_count, [](KeyEntry l, KeyEntry r) {
                            if (l.key_ == r.key_) {
                                return l.value_offset_ > r.value_offset_;
                            } else {
                                return l.key_ < r.key_;
                            }
                        });
                    }
                }
            }));
        }
        for (uint32_t i = 0; i < NUM_READ_KEY_THREADS; ++i) {
            workers[i].join();
            free(local_buffers_g[i]);
        }
        delete[]local_buffers_g;
        clock_end = high_resolution_clock::now();
        log_info("Finish BI, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() /
                 1000.0);

    }
}  // namespace polar_race
