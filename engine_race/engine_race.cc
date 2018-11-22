#include <utility>

// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <malloc.h>

#include <cassert>

#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

#include <byteswap.h>

#include "log.h"
#include "stat.h"

#define STAT
//#define BARRIER_FOR_CACHE

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

    inline uint32_t get_key_par_id(uint64_t key) {
        return static_cast<uint32_t >((key >> (NUM_THREADS - KEY_BUCKET_DIGITS)) & 0xffffffu);
    }

    inline uint32_t get_val_par_id(uint64_t key) {
        return static_cast<uint32_t >((key >> (NUM_THREADS - VAL_BUCKET_DIGITS)) & 0xffffffu);
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

    const string value_meta_file_name = "polar.keymeta";
    const string key_meta_file_name = "polar.valmeta";
    const string value_file_name = "polar.values";
    const string value_buffer_file_name = "polar.valbuffers";
    const string key_buffer_file_name = "polar.keybuffers";

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
            mmap_key_meta_cnt_(nullptr), mmap_val_meta_cnt_(nullptr),
            write_key_file_dp_(nullptr), write_key_buffer_file_dp_(nullptr),
            mmap_key_aligned_buffer_(nullptr), key_mtx_(nullptr), val_mtx_(nullptr),
            write_value_file_dp_(nullptr), write_value_buffer_file_dp_(nullptr),
            mmap_value_aligned_buffer_(nullptr), aligned_read_buffer_(nullptr),
            barrier_(WRITE_BARRIER_NUM), read_barrier_(READ_BARRIER_NUM), range_barrier_ptr_(nullptr),
            is_sorted_(nullptr), is_range_init_(false), polar_str_pairs_(NUM_THREADS),
            shared_buffer_(nullptr),
            total_time_(0), wait_get_time_(0), enqueue_time_(0),
            val_buffer_max_size_(0), range_io_worker_pool_(nullptr),
            bucket_mutex_arr_(nullptr), bucket_cond_var_arr_(nullptr),
            bucket_is_ready_read_(nullptr), bucket_consumed_num_(nullptr), total_range_num_threads_(0) {
        clock_end = high_resolution_clock::now();
        log_info("Start init DB, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        const string val_meta_file_path = dir + "/" + value_meta_file_name;
        const string key_meta_file_path = dir + "/" + key_meta_file_name;

        const string key_file_path = dir + "/" + key_meta_file_name;
        const string tmp_key_file_path = dir + "/" + key_buffer_file_name;

        const string value_file_path = dir + "/" + value_file_name;
        const string tmp_value_file_path = dir + "/" + value_buffer_file_name;

        write_key_file_dp_ = new int[KEY_BUCKET_NUM];
        write_key_buffer_file_dp_ = new int[KEY_BUCKET_NUM];
        mmap_key_aligned_buffer_ = new KeyEntry *[KEY_BUCKET_NUM];

        write_value_file_dp_ = new int[VAL_BUCKET_NUM];
        write_value_buffer_file_dp_ = new int[VAL_BUCKET_NUM];
        mmap_value_aligned_buffer_ = new char *[VAL_BUCKET_NUM];

        if (!file_exists(val_meta_file_path.c_str())) {
            log_info("Initialize the database...");
            // Meta.
            val_meta_file_dp_ = open(val_meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            ftruncate(val_meta_file_dp_, sizeof(uint32_t) * VAL_BUCKET_NUM);
            mmap_val_meta_cnt_ = (uint32_t *) mmap(nullptr, sizeof(uint32_t) * (VAL_BUCKET_NUM),
                                                   PROT_READ | PROT_WRITE, MAP_SHARED, val_meta_file_dp_, 0);
            memset(mmap_val_meta_cnt_, 0, sizeof(uint32_t) * (VAL_BUCKET_NUM));

            key_meta_file_dp_ = open(key_meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            ftruncate(key_meta_file_dp_, sizeof(uint32_t) * KEY_BUCKET_NUM);
            mmap_key_meta_cnt_ = (uint32_t *) mmap(nullptr, sizeof(uint32_t) * (KEY_BUCKET_NUM),
                                                   PROT_READ | PROT_WRITE, MAP_SHARED, key_meta_file_dp_, 0);
            memset(mmap_key_meta_cnt_, 0, sizeof(uint32_t) * (KEY_BUCKET_NUM));

            // Value.
            val_mtx_ = new mutex[VAL_BUCKET_NUM];
            for (int i = 0; i < VAL_BUCKET_NUM; ++i) {
                string temp_value = value_file_path + to_string(i);
                string temp_buffer_value = tmp_value_file_path + to_string(i);

                write_value_file_dp_[i] = open(temp_value.c_str(), O_RDWR | O_CREAT | O_DIRECT, FILE_PRIVILEGE);
                if (write_value_file_dp_[i] < 0) {
                    log_info("fd err of %d: %d, err info: %s", i, write_value_file_dp_[i], strerror(errno));
                    exit(-1);
                }
//                ftruncate(write_value_file_dp_[i], static_cast<uint64_t>(VALUE_SIZE) * (TOTAL_COUNT / VAL_BUCKET_NUM));
                fallocate(write_value_file_dp_[i], 0, 0,
                          static_cast<uint64_t>(VALUE_SIZE) * (72000000 / VAL_BUCKET_NUM));

                size_t tmp_buffer_value_file_size = VALUE_SIZE * TMP_VALUE_BUFFER_SIZE;
                write_value_buffer_file_dp_[i] = open(temp_buffer_value.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                if (write_value_buffer_file_dp_[i] < 0) {
                    log_info("fd err of %d: %d, err info: %s", i, write_value_buffer_file_dp_[i], strerror(errno));
                    exit(-1);
                }
                ftruncate(write_value_buffer_file_dp_[i], tmp_buffer_value_file_size);
                mmap_value_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_value_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, write_value_buffer_file_dp_[i], 0);
            }

            // Key.
            key_mtx_ = new mutex[KEY_BUCKET_NUM];
            for (int i = 0; i < KEY_BUCKET_NUM; ++i) {
                string temp_key = key_file_path + to_string(i);
                string temp_buffer_key = tmp_key_file_path + to_string(i);
                write_key_file_dp_[i] = open(temp_key.c_str(), O_RDWR | O_CREAT | O_DIRECT, FILE_PRIVILEGE);
                ftruncate(write_key_file_dp_[i],
                          static_cast<uint64_t>(sizeof(KeyEntry)) * (TOTAL_COUNT / KEY_BUCKET_NUM));

                constexpr size_t tmp_buffer_key_file_size = sizeof(KeyEntry) * (size_t) TMP_KEY_BUFFER_SIZE;
                write_key_buffer_file_dp_[i] = open(temp_buffer_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                ftruncate(write_key_buffer_file_dp_[i], tmp_buffer_key_file_size);
                mmap_key_aligned_buffer_[i] = (KeyEntry *) mmap(nullptr, tmp_buffer_key_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, write_key_buffer_file_dp_[i], 0);
            }
            log_info("Create the database successfully.");
        } else {
            log_info("Reload the database.");
            // Meta.
            val_meta_file_dp_ = open(val_meta_file_path.c_str(), O_RDONLY, FILE_PRIVILEGE);
            mmap_val_meta_cnt_ = (uint32_t *) mmap(nullptr, sizeof(uint32_t) * (VAL_BUCKET_NUM),
                                                   PROT_READ, MAP_PRIVATE | MAP_POPULATE, val_meta_file_dp_, 0);
            key_meta_file_dp_ = open(key_meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            mmap_key_meta_cnt_ = (uint32_t *) mmap(nullptr, sizeof(uint32_t) * (KEY_BUCKET_NUM),
                                                   PROT_READ | PROT_WRITE, MAP_SHARED, key_meta_file_dp_, 0);

            // Value.
            for (int i = 0; i < VAL_BUCKET_NUM; ++i) {
                string temp_value = value_file_path + to_string(i);
                string temp_buffer_value = tmp_value_file_path + to_string(i);

                write_value_file_dp_[i] = open(temp_value.c_str(), O_RDWR | O_DIRECT, FILE_PRIVILEGE);
                write_value_buffer_file_dp_[i] = open(temp_buffer_value.c_str(), O_RDWR, FILE_PRIVILEGE);

                size_t tmp_buffer_value_file_size = VALUE_SIZE * TMP_VALUE_BUFFER_SIZE;
                mmap_value_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_value_file_size,
                                                              PROT_READ | PROT_WRITE, MAP_SHARED,
                                                              write_value_buffer_file_dp_[i], 0);
            }
            // Key.
            key_mtx_ = new mutex[KEY_BUCKET_NUM];
            for (int i = 0; i < KEY_BUCKET_NUM; ++i) {
                string temp_key = key_file_path + to_string(i);
                string temp_buffer_key = tmp_key_file_path + to_string(i);
                write_key_file_dp_[i] = open(temp_key.c_str(), O_RDWR, FILE_PRIVILEGE);

                write_key_buffer_file_dp_[i] = open(temp_buffer_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                constexpr size_t tmp_buffer_key_file_size = sizeof(KeyEntry) * (size_t) TMP_KEY_BUFFER_SIZE;
                mmap_key_aligned_buffer_[i] = (KeyEntry *) mmap(nullptr, tmp_buffer_key_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, write_key_buffer_file_dp_[i], 0);

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
        // Meta.
        if (mmap_val_meta_cnt_ != nullptr) {
            munmap(mmap_val_meta_cnt_, sizeof(uint32_t) * (VAL_BUCKET_NUM));
        }
        close(val_meta_file_dp_);
        if (mmap_key_meta_cnt_ != nullptr) {
            munmap(mmap_key_meta_cnt_, sizeof(uint32_t) * (KEY_BUCKET_NUM));
        }
        close(key_meta_file_dp_);

        // Thread.
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            if (aligned_read_buffer_ != nullptr) {
                free(aligned_read_buffer_[i]);
            }
        }
        delete[] aligned_read_buffer_;

        // Key.
        for (uint32_t i = 0; i < KEY_BUCKET_NUM; ++i) {
            if (mmap_key_aligned_buffer_[i] != nullptr) {
                munmap(mmap_key_aligned_buffer_[i], sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE);
            }
            close(write_key_file_dp_[i]);
            close(write_key_buffer_file_dp_[i]);
        }
        delete[] write_key_file_dp_;
        delete[] mmap_key_aligned_buffer_;
        delete[] key_mtx_;
        delete[] is_sorted_;
        // Free indices
        for (KeyEntry *index_partition: index_) {
            free(index_partition);
        }
        free(shared_buffer_);

        // Value.
        for (uint32_t i = 0; i < VAL_BUCKET_NUM; ++i) {
            if (mmap_value_aligned_buffer_[i] != nullptr) {
                munmap(mmap_value_aligned_buffer_[i], VALUE_SIZE * (size_t) TMP_VALUE_BUFFER_SIZE);
            }
            close(write_value_file_dp_[i]);
            close(write_value_buffer_file_dp_[i]);
        }
        delete[] write_value_file_dp_;
        delete[] mmap_value_aligned_buffer_;

        // Range: Thread.
        if (is_range_init_) {
            for (auto &kv_pair : polar_str_pairs_) {
                if (kv_pair.first != nullptr)
                    delete[] kv_pair.first->data();
                delete kv_pair.first;
                if (kv_pair.second != nullptr)
                    delete[] kv_pair.second->data();
                delete kv_pair.second;
            }
        }
        delete range_barrier_ptr_;
        for (char *ptr: io_buffers_) {
            free(ptr);
        }
        delete range_io_worker_pool_;

        if (total_time_ != 0) {
            log_info("Total Range Time: %.9lf s, wait: %.9lf s, enqueue: %.9lf s", total_time_, wait_get_time_,
                     enqueue_time_);
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
        uint64_t val_par_id = get_val_par_id(key_int_big_endian);
#ifdef STAT
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> first_write_clk;
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> last_write_clk;
        if (local_block_offset == 0) {
            first_write_clk = high_resolution_clock::now();
        }
#endif
        if (local_block_offset % 10000 == 0 && tid < WRITE_BARRIER_NUM) {
            barrier_.Wait();
        }

        // Value.
        uint32_t val_offset_get;
        {
            unique_lock<mutex> lock(val_mtx_[val_par_id]);
            val_offset_get = mmap_val_meta_cnt_[val_par_id];
            // Write value to the value file, with a tmp file as value_buffer.
            uint32_t val_buffer_offset = (mmap_val_meta_cnt_[val_par_id] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
            char *value_buffer = mmap_value_aligned_buffer_[val_par_id];
            memcpy(value_buffer + val_buffer_offset, value.data(), VALUE_SIZE);
            if ((mmap_val_meta_cnt_[val_par_id] + 1) % TMP_VALUE_BUFFER_SIZE == 0) {
                pwrite(write_value_file_dp_[val_par_id], value_buffer, VALUE_SIZE * TMP_VALUE_BUFFER_SIZE,
                       ((uint64_t) mmap_val_meta_cnt_[val_par_id] - (TMP_VALUE_BUFFER_SIZE - 1)) * VALUE_SIZE);
            }
            // Update the meta data.
            mmap_val_meta_cnt_[val_par_id]++;
        }
        // Key.
        {
            unique_lock<mutex> lock(key_mtx_[key_par_id]);
            // Write key to the key file.
            uint32_t key_buffer_offset = (mmap_key_meta_cnt_[key_par_id] % TMP_KEY_BUFFER_SIZE);
            KeyEntry *key_buffer = mmap_key_aligned_buffer_[key_par_id];
            key_buffer[key_buffer_offset].key_ = key_int_big_endian;
            key_buffer[key_buffer_offset].value_offset_ = val_offset_get;
            if (((mmap_key_meta_cnt_[key_par_id] + 1) % TMP_KEY_BUFFER_SIZE) == 0) {
                pwrite(write_key_file_dp_[key_par_id], key_buffer, sizeof(KeyEntry) * TMP_KEY_BUFFER_SIZE,
                       ((uint64_t) mmap_key_meta_cnt_[key_par_id] - (TMP_KEY_BUFFER_SIZE - 1)) * sizeof(KeyEntry));
            }
            // Update the meta data.
            mmap_key_meta_cnt_[key_par_id]++;
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

#ifdef POSTPONE_READ
        LazyLoadIndex(key_par_id);
#endif
        auto it = index_[key_par_id] + branchfree_search(index_[key_par_id], mmap_key_meta_cnt_[key_par_id], tmp);
        local_block_offset++;

        if (local_block_offset % 100000 == 0 && tid < READ_BARRIER_NUM) {
            read_barrier_.Wait();
        }
        if (it == index_[key_par_id] + mmap_key_meta_cnt_[key_par_id] || it->key_ != big_endian_key_uint) {
            if (is_first_not_found) {
                log_info("not found in tid: %d\n", tid);
                is_first_not_found = false;
            }
            return kNotFound;
        }

        uint64_t val_par_id = get_val_par_id(big_endian_key_uint);
        pread(write_value_file_dp_[val_par_id], value_buffer, VALUE_SIZE,
              static_cast<uint64_t>(it->value_offset_) * VALUE_SIZE);

        value->assign(value_buffer, VALUE_SIZE);
        return kSucc;
    }

    void EngineRace::ReadBucketToBuffer(uint32_t bucket_id) {
        auto range_clock_beg = high_resolution_clock::now();
        if (bucket_id % 256 == 0) {
            log_info("In bucket %d, ReadBucketToBuffer start ts: %.9lf s", bucket_id,
                     std::chrono::duration_cast<std::chrono::nanoseconds>(range_clock_beg.time_since_epoch()).count() /
                     1000000000.0);
        }
        auto buffer_id = static_cast<uint32_t>(bucket_id % MAX_BUFFER_NUM);

        auto ret = pread(write_value_file_dp_[bucket_id], io_buffers_[buffer_id],
                         static_cast<uint64_t >(VALUE_SIZE) * mmap_val_meta_cnt_[bucket_id], 0);
        if (ret < 0) {
            log_info("in range read err: %s, size: %zu", strerror(errno));
        }
        auto range_clock_end = high_resolution_clock::now();
        double elapsed_time = duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                              static_cast<double>(1000000000);
        if (bucket_id % 256 == 0) {
            log_info("In bucket %d, ReadBucketToBuffer elapsed read time %.9lf s, ts: %.9lf s", bucket_id, elapsed_time,
                     std::chrono::duration_cast<std::chrono::nanoseconds>(range_clock_end.time_since_epoch()).count() /
                     1000000000.0);
        }
        total_time_ += elapsed_time;
    }

    void EngineRace::InitForRange(int64_t tid) {
        if (!is_range_init_) {
            unique_lock<mutex> lock(range_mtx_);
            if (tid == 0) {
                range_io_worker_pool_ = new ThreadPool(1);

                for (int i = 0; i < VAL_BUCKET_NUM; i++) {
                    val_buffer_max_size_ = max<uint64_t>(val_buffer_max_size_, mmap_val_meta_cnt_[i]);
                }
                val_buffer_max_size_ *= VALUE_SIZE;
                log_info("Max Buffer Size: %zu", val_buffer_max_size_);
                io_buffers_ = vector<char *>(MAX_BUFFER_NUM);
                shared_buffer_ = (char *) malloc(val_buffer_max_size_);
                for (int i = 0; i < MAX_BUFFER_NUM; i++) {
                    io_buffers_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, val_buffer_max_size_);
                }
                const string value_file_path = dir_ + "/" + value_file_name;
                // Value Files.
                for (int i = 0; i < VAL_BUCKET_NUM; i++) {
                    string temp_value = value_file_path + to_string(i);
                    close(write_value_file_dp_[i]);
                    write_value_file_dp_[i] = open(temp_value.c_str(), O_RDONLY | O_DIRECT, FILE_PRIVILEGE);
                }
                // Sleep here
                auto range_clock_beg = high_resolution_clock::now();
                bucket_mutex_arr_ = new mutex[VAL_BUCKET_NUM];
                bucket_cond_var_arr_ = new condition_variable[VAL_BUCKET_NUM];
                bucket_is_ready_read_ = new bool[VAL_BUCKET_NUM];
                bucket_consumed_num_ = new atomic_int[VAL_BUCKET_NUM];
                futures_.resize(VAL_BUCKET_NUM);
                usleep(10000);
                auto range_clock_end = high_resolution_clock::now();
                double elapsed_time = duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                                      static_cast<double>(1000000000);
                log_info("elapsed read time in first sync, %.9lf s", 0, elapsed_time);

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
            memset(bucket_is_ready_read_, 0, VAL_BUCKET_NUM);
            for (uint32_t i = 0; i < MAX_BUFFER_NUM; i++) {
                bucket_is_ready_read_[i] = true;
            }
            for (auto next_bucket_idx = 0; next_bucket_idx < VAL_BUCKET_NUM; next_bucket_idx++) {
                bucket_consumed_num_[next_bucket_idx].store(0);
//                log_info("next bucket id: %d", next_bucket_idx);
                futures_[next_bucket_idx] = range_io_worker_pool_->enqueue([this, next_bucket_idx]() {
                    unique_lock<mutex> lock(bucket_mutex_arr_[next_bucket_idx]);
                    if (!bucket_is_ready_read_[next_bucket_idx]) {
                        bucket_cond_var_arr_[next_bucket_idx].wait(lock, [this, next_bucket_idx]() {
                            return bucket_is_ready_read_[next_bucket_idx];
                        });
                    }
                    ReadBucketToBuffer(next_bucket_idx);
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
        static thread_local PolarString *polar_val_ptr_;

        auto range_init_start_clock = high_resolution_clock::now();

        log_info("Start range ts: %.9lf s in tid %d",
                 std::chrono::duration_cast<std::chrono::nanoseconds>(
                         range_init_start_clock.time_since_epoch()).count() / 1000000000.0, tid);
        // Thread Local Key/Value Init.
        if (local_block_offset == 0) {
            char *key_chars = new char[sizeof(uint64_t)];
            char *val_chars = new char[VALUE_SIZE];
            polar_str_pairs_[tid].first = new PolarString(key_chars, sizeof(uint64_t));
            polar_key_ptr_ = polar_str_pairs_[tid].first;
            polar_str_pairs_[tid].second = new PolarString(val_chars, VALUE_SIZE);
            polar_val_ptr_ = polar_str_pairs_[tid].second;
        }
        uint64_t key_lower_uint64 = polar_str_to_big_endian_uint64(lower);
        uint64_t key_upper_uint64 = polar_str_to_big_endian_uint64(upper);
        if (invocation_num < 10) {
            invocation_num++;
            log_info("tid: %d, [%zu, %zu), sizes: %zu, %zu, invocation times: %d", tid, key_lower_uint64,
                     key_upper_uint64, lower.size(), upper.size(), invocation_num);
        }
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
        uint32_t upper_key_par_id = KEY_BUCKET_NUM - 1;
        for (uint32_t key_par_id = lower_key_par_id; key_par_id < upper_key_par_id + 1; key_par_id++) {
            if (tid == 0) {
                auto wait_start_clock = high_resolution_clock::now();
                futures_[key_par_id].get();
                auto wait_end_clock = high_resolution_clock::now();
                elapsed_time = duration_cast<nanoseconds>(wait_end_clock - wait_start_clock).count() /
                               static_cast<double>(1000000000);
                wait_get_time_ += elapsed_time;
            }
            range_barrier_ptr_->Wait();
            {
                // Parallel Gathering.
                uint32_t inner_loop_buffer_idx = key_par_id % MAX_BUFFER_NUM;
                uint32_t total_gather_cnt = mmap_key_meta_cnt_[key_par_id];
                uint32_t avg = total_gather_cnt / total_range_num_threads_ + 1;
                uint32_t my_end = std::min<uint32_t>(total_gather_cnt, avg * (tid + 1));
                for (uint32_t in_par_id = avg * tid; in_par_id < my_end; in_par_id++) {
                    uint64_t val_id = index_[key_par_id][in_par_id].value_offset_;
                    memcpy(shared_buffer_ + VALUE_SIZE * in_par_id,
                           io_buffers_[inner_loop_buffer_idx] + val_id * VALUE_SIZE, VALUE_SIZE);
                }
            }
            range_barrier_ptr_->Wait();
            if (tid == 0) {
                // Notify.
                uint32_t next_bucket_idx = key_par_id + MAX_BUFFER_NUM;
                if (next_bucket_idx < upper_key_par_id + 1) {
                    unique_lock<mutex> lock(bucket_mutex_arr_[next_bucket_idx]);
                    bucket_is_ready_read_[next_bucket_idx] = true;
                    bucket_cond_var_arr_[next_bucket_idx].notify_all();
                }
            }

            uint32_t in_par_id_beg = 0;
            uint32_t in_par_id_end = mmap_key_meta_cnt_[key_par_id];
            uint64_t prev_key = 0;
            for (uint32_t in_par_id = 0; in_par_id < in_par_id_end; in_par_id++) {
                // Skip the equalities.
                uint64_t big_endian_key = index_[key_par_id][in_par_id].key_;
                if (in_par_id != in_par_id_beg) {
                    if (big_endian_key == prev_key) {
                        continue;
                    }
                }
                prev_key = big_endian_key;

                // Key (to little endian first).
                (*(uint64_t *) polar_key_ptr_->data()) = bswap_64(big_endian_key);

                // Value.
                memcpy((char *) polar_val_ptr_->data(), shared_buffer_ + in_par_id * VALUE_SIZE, VALUE_SIZE);
                // Visit Key/Value.
                visitor.Visit(*polar_key_ptr_, *polar_val_ptr_);
            }
        }
        if (tid == 0) { log_info("one round ok..."); }
        range_barrier_ptr_->Wait();
        return kSucc;
    }

    void EngineRace::FlushTmpFiles(string dir) {
        // Flush Values.
        for (int i = 0; i < VAL_BUCKET_NUM; i++) {
            const string value_file_path = dir + "/" + value_file_name;
            string temp_value = value_file_path + to_string(i);
            if ((mmap_val_meta_cnt_[i] % TMP_VALUE_BUFFER_SIZE) != 0) {
                size_t write_length = (mmap_val_meta_cnt_[i] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                size_t write_offset = static_cast<uint64_t>(mmap_val_meta_cnt_[i] / TMP_VALUE_BUFFER_SIZE *
                                                            TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                auto tmp_fd = open(temp_value.c_str(), O_RDWR, FILE_PRIVILEGE);
                pwrite(tmp_fd, mmap_value_aligned_buffer_[i], write_length, write_offset);
                close(tmp_fd);
            }
        }
        // Flush Keys.
        for (int i = 0; i < KEY_BUCKET_NUM; i++) {
            const string key_file_path = dir + "/" + key_meta_file_name;
            string temp_key = key_file_path + to_string(i);
            if ((mmap_key_meta_cnt_[i] % TMP_KEY_BUFFER_SIZE) != 0) {
                size_t write_length = (mmap_key_meta_cnt_[i] % TMP_KEY_BUFFER_SIZE) * sizeof(KeyEntry);
                size_t write_offset = static_cast<uint64_t>(mmap_key_meta_cnt_[i] / TMP_KEY_BUFFER_SIZE *
                                                            TMP_KEY_BUFFER_SIZE) * sizeof(KeyEntry);
                auto tmp_fd = open(temp_key.c_str(), O_RDWR, FILE_PRIVILEGE);
                pwrite(tmp_fd, mmap_key_aligned_buffer_[i], write_length, write_offset);
                close(tmp_fd);
            }
        }
    }

    void EngineRace::LazyLoadIndex(uint32_t key_par_id) {
        if (!is_sorted_[key_par_id] && mmap_key_meta_cnt_[key_par_id] > 0) {
            unique_lock<mutex> lock(key_mtx_[key_par_id]);
            if (!is_sorted_[key_par_id]) {
                auto ret = pread(write_key_file_dp_[key_par_id], index_[key_par_id],
                                 mmap_key_meta_cnt_[key_par_id] * sizeof(KeyEntry), 0);
                if (ret != mmap_key_meta_cnt_[key_par_id] * sizeof(KeyEntry)) {
                    log_info("ret: %d, err: %s", ret, strerror(errno));
                }
//                assert(ret == mmap_key_meta_cnt_[key_par_id] * sizeof(KeyEntry));
                sort(index_[key_par_id], index_[key_par_id] + mmap_key_meta_cnt_[key_par_id],
                     [](KeyEntry l, KeyEntry r) {
                         if (l.key_ == r.key_) {
                             return l.value_offset_ > r.value_offset_;
                         } else {
                             return l.key_ < r.key_;
                         }
                     });
                is_sorted_[key_par_id] = true;
            }
        }
    }

    void EngineRace::BuildIndex(string dir) {
        log_info("Begin BI, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        // Key Cnt, Index Allocation.
        is_sorted_ = new bool[KEY_BUCKET_NUM];
        memset((void *) is_sorted_, 0, KEY_BUCKET_NUM);
        index_ = vector<KeyEntry *>(KEY_BUCKET_NUM, nullptr);
        for (int key_par_id = 0; key_par_id < KEY_BUCKET_NUM; key_par_id++) {
            index_[key_par_id] = static_cast<KeyEntry *>(malloc(mmap_key_meta_cnt_[key_par_id] * sizeof(KeyEntry)));
        }

        // Flush tmp Files
        FlushTmpFiles(std::move(dir));

#ifndef POSTPONE_READ
        // Read each key file.
        vector<thread> workers(NUM_THREADS);
        for (uint32_t tid = 0; tid < NUM_THREADS; ++tid) {
            workers[tid] = move(thread([tid, this]() {
                for (uint32_t key_par_id = tid; key_par_id < KEY_BUCKET_NUM; key_par_id += NUM_THREADS) {
                    if (mmap_key_meta_cnt_[key_par_id] > 0) {
                        auto ret = pread(write_key_file_dp_[key_par_id], index_[key_par_id],
                                         mmap_key_meta_cnt_[key_par_id] * sizeof(KeyEntry), 0);
                        if (ret != mmap_key_meta_cnt_[key_par_id] * sizeof(KeyEntry)) {
                            log_info("ret: %d, err: %s", ret, strerror(errno));
                        }
                        assert(ret == mmap_key_meta_cnt_[key_par_id] * sizeof(KeyEntry));
                        sort(index_[key_par_id], index_[key_par_id] + mmap_key_meta_cnt_[key_par_id],
                             [](KeyEntry l, KeyEntry r) {
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
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            workers[i].join();
        }
#endif
        clock_end = high_resolution_clock::now();
        log_info("Finish BI, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
    }
}  // namespace polar_race
