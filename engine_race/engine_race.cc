#include <utility>

// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <malloc.h>

#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

#include <byteswap.h>

#include "log.h"
#include "util.h"
#include "file_util.h"

//#define STAT
//#define DSTAT_TESTING
//#define RANGE_STAT

namespace polar_race {
    using namespace std;

    atomic_int write_num_threads(-1);
    atomic_int read_num_threads_count(-1);
    atomic_int range_num_threads_count(-1);

    const string key_file_name = "polar.keys";
    const string value_file_name = "polar.values";

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

    inline uint32_t get_par_bucket_id(uint64_t key) {
        return static_cast<uint32_t >((key >> (NUM_THREADS - BUCKET_DIGITS)) & 0xffffffu);
    }

    inline uint32_t get_buffer_id(uint32_t bucket_id) {
        if (bucket_id >= KEEP_REUSE_BUFFER_NUM) {
            return (bucket_id % MAX_RECYCLE_BUFFER_NUM) + KEEP_REUSE_BUFFER_NUM;
        } else {
            return bucket_id;
        }
    }

#ifdef RANGE_STAT

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

#endif

    RetCode Engine::Open(const std::string &name, Engine **eptr) {
        clock_start = high_resolution_clock::now();
        log_info("sizeof %d, %d, %d", sizeof(off_t), sizeof(off64_t), sizeof(KeyEntry));
        auto ret = EngineRace::Open(name, eptr);
        clock_end = high_resolution_clock::now();
        log_info("After open DB, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        return ret;
    }

    Engine::~Engine() = default;

    /*
 * Complete the functions below to implement you own engine
 */
    EngineRace::EngineRace(const std::string &dir) :
            mmap_meta_cnt_(nullptr), key_file_dp_(nullptr), key_buffer_file_dp_(nullptr),
            mmap_key_aligned_buffer_(nullptr), value_file_dp_(nullptr), value_buffer_file_dp_(nullptr),
            mmap_value_aligned_buffer_(nullptr),
            fallocate_pool_(nullptr), partition_mtx_(nullptr), write_barrier_(WRITE_BARRIER_NUM),
            aligned_read_buffer_(nullptr), read_barrier_(READ_BARRIER_NUM),
            is_range_init_(false), range_barrier_ptr_(nullptr), polar_keys_(NUM_THREADS),
            total_time_(0), total_io_sleep_time_(0), wait_get_time_(0),
            val_buffer_max_size_(0), range_io_worker_pool_(nullptr),
            bucket_mutex_arr_(nullptr), bucket_cond_var_arr_(nullptr),
            bucket_is_ready_read_(nullptr), bucket_consumed_num_(nullptr), total_range_num_threads_(0) {
        printTS(__FUNCTION__, __LINE__, clock_start);

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
            // Meta.
            meta_cnt_file_dp_ = open(meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
            ftruncate(meta_cnt_file_dp_, sizeof(uint32_t) * BUCKET_NUM);
            mmap_meta_cnt_ = (uint32_t *) mmap(nullptr, sizeof(uint32_t) * (BUCKET_NUM),
                                               PROT_READ | PROT_WRITE, MAP_SHARED, meta_cnt_file_dp_, 0);
            memset(mmap_meta_cnt_, 0, sizeof(uint32_t) * (BUCKET_NUM));

            // Reserve FAllocate Value File Jobs.
            fallocate_pool_ = new ThreadPool(FALLOCATE_POOL_SIZE);
            fallocate_futures_per_bucket_.resize(BUCKET_NUM);
            fallocate_slice_id_end_.resize(BUCKET_NUM, 0);

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

                // Submit FAllocate Value File Jobs.
                for (int j = 0; j < MAX_FALLOCATE_RESERVE_SLICE_NUM; j++) {
                    fallocate_futures_per_bucket_[i].push(fallocate_pool_->enqueue([this, i]() {
                        fallocate(value_file_dp_[i], 0, FALLOCATE_SIZE, fallocate_slice_id_end_[i] * FALLOCATE_SIZE);
                    }));
                    fallocate_slice_id_end_[i]++;
                }
            }

            // Key.
            for (int i = 0; i < BUCKET_NUM; ++i) {
                string temp_key = key_file_path + to_string(i);
                string temp_buffer_key = tmp_key_file_path + to_string(i);
                key_file_dp_[i] = open(temp_key.c_str(), O_RDWR | O_CREAT | O_DIRECT, FILE_PRIVILEGE);
                fallocate(key_file_dp_[i], 0, 0, FALLOCATE_KEY_FILE_SIZE);

                constexpr size_t tmp_buffer_key_file_size = sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE;
                key_buffer_file_dp_[i] = open(temp_buffer_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                ftruncate(key_buffer_file_dp_[i], tmp_buffer_key_file_size);
                mmap_key_aligned_buffer_[i] = (uint64_t *) mmap(nullptr, tmp_buffer_key_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, key_buffer_file_dp_[i], 0);
            }
        } else {
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

                key_buffer_file_dp_[i] = -1;
                mmap_key_aligned_buffer_[i] = nullptr;
            }
            // Thread.
            aligned_read_buffer_ = new char *[NUM_THREADS];
            for (int i = 0; i < NUM_THREADS; ++i) {
                aligned_read_buffer_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, VALUE_SIZE);
            }

            // Flush tmp Files.
            FlushTmpFiles(dir);
            // Build Index.
            BuildIndex();
        }
        clock_end = high_resolution_clock::now();
        log_info("After init DB, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        printTS(__FUNCTION__, __LINE__, clock_start);
    }

// 1. Open engine
    RetCode EngineRace::Open(const std::string &name, Engine **eptr) {
        printTS(__FUNCTION__, __LINE__, clock_start);
#ifdef DSTAT_TESTING
        PrintMemFree();
        DstatCorountine();
        IOStatCoroutine();
#endif
        if (!file_exists(name.c_str())) {
            int ret = mkdir(name.c_str(), 0755);
            if (ret != 0) {
                log_info("Fail to create the target directory %s.", name.c_str());
                exit(-1);
            }
            log_info("Create the target directory %s.", name.c_str());
        }
        *eptr = new EngineRace(name);
        printTS(__FUNCTION__, __LINE__, clock_start);
        return kSucc;
    }

    EngineRace::~EngineRace() {
        printTS(__FUNCTION__, __LINE__, clock_start);
        clock_end = high_resolution_clock::now();
        log_info("Start ~EngineRace(), time: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0);
#ifdef DSTAT_TESTING
        PrintMemFree();
#endif
        delete fallocate_pool_;
        // Thread.
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            if (aligned_read_buffer_ != nullptr) {
                free(aligned_read_buffer_[i]);
            }
        }
        delete[] aligned_read_buffer_;

        // Key.
        for (uint32_t i = 0; i < BUCKET_NUM; ++i) {
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

        // Value.
        for (uint32_t i = 0; i < BUCKET_NUM; ++i) {
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

        // Free indices.
        for (KeyEntry *index_partition: index_) {
            free(index_partition);
        }

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
#ifdef DSTAT_TESTING
        PrintMemFree();
#endif
        clock_end = high_resolution_clock::now();
        log_info("Finish ~EngineRace(), time: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0);
        printTS(__FUNCTION__, __LINE__, clock_start);
    }

// 3. Write a key-value pair into engine
    void EngineRace::FAllocateForBucket(uint32_t par_bucket_id) {
        if ((mmap_meta_cnt_[par_bucket_id] * VALUE_SIZE) % (FALLOCATE_SIZE) == 0) {
            fallocate_futures_per_bucket_[par_bucket_id].front().get();
            fallocate_futures_per_bucket_[par_bucket_id].pop();

            // Submit FAllocate Job.
            fallocate_futures_per_bucket_[par_bucket_id].push(fallocate_pool_->enqueue([this, par_bucket_id]() {
                int ret = fallocate(value_file_dp_[par_bucket_id], 0, FALLOCATE_SIZE,
                                    fallocate_slice_id_end_[par_bucket_id] * FALLOCATE_SIZE);
                if (ret < 0) {
                    log_info("Fallocate Err For Bucket %d, slice id: %d, Ret: %d, Err: %s", par_bucket_id,
                             fallocate_slice_id_end_[par_bucket_id], ret, strerror(errno));
                }
            }));
            fallocate_slice_id_end_[par_bucket_id]++;
        }
    }

    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local uint32_t tid = (uint32_t) (++write_num_threads) % NUM_THREADS;
        static thread_local uint32_t local_block_offset = 0;
        uint64_t key_int_big_endian = bswap_64(TO_UINT64(key.data()));
        uint32_t par_bucket_id = get_par_bucket_id(key_int_big_endian);
#ifdef STAT
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> first_write_clk;
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> last_write_clk;
        if (local_block_offset == 0) {
            first_write_clk = high_resolution_clock::now();
        }
#endif
#ifdef ENABLE_WRITE_BARRIER
        if (local_block_offset % 10000 == 0 && local_block_offset < 900000 && tid < WRITE_BARRIER_NUM) {
            write_barrier_.Wait();
        }
#endif
        {
            unique_lock<mutex> lock(partition_mtx_[par_bucket_id]);
            // Write value to the value file, with a tmp file as value_buffer.
            uint32_t val_buffer_offset = (mmap_meta_cnt_[par_bucket_id] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
            char *value_buffer = mmap_value_aligned_buffer_[par_bucket_id];
            memcpy(value_buffer + val_buffer_offset, value.data(), VALUE_SIZE);

            // Allocate disk space.
            FAllocateForBucket(par_bucket_id);

            // Write value to the value file.
            if ((mmap_meta_cnt_[par_bucket_id] + 1) % TMP_VALUE_BUFFER_SIZE == 0) {
                uint64_t write_offset =
                        ((uint64_t) mmap_meta_cnt_[par_bucket_id] - (TMP_VALUE_BUFFER_SIZE - 1)) * VALUE_SIZE;
                pwrite(value_file_dp_[par_bucket_id], value_buffer, VALUE_SIZE * TMP_VALUE_BUFFER_SIZE, write_offset);
            }
            // Write key to the key file.
            uint32_t key_buffer_offset = (mmap_meta_cnt_[par_bucket_id] % TMP_KEY_BUFFER_SIZE);
            uint64_t *key_buffer = mmap_key_aligned_buffer_[par_bucket_id];
            key_buffer[key_buffer_offset] = key_int_big_endian;
            if (((mmap_meta_cnt_[par_bucket_id] + 1) % TMP_KEY_BUFFER_SIZE) == 0) {
                pwrite(key_file_dp_[par_bucket_id], key_buffer, sizeof(uint64_t) * TMP_KEY_BUFFER_SIZE,
                       ((uint64_t) mmap_meta_cnt_[par_bucket_id] - (TMP_KEY_BUFFER_SIZE - 1)) * sizeof(uint64_t));
            }
            // Update the meta data.
            mmap_meta_cnt_[par_bucket_id]++;
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
        auto bucket_id = get_par_bucket_id(big_endian_key_uint);

        auto it = index_[bucket_id] + branchfree_search(index_[bucket_id], mmap_meta_cnt_[bucket_id], tmp);
        local_block_offset++;

        if (local_block_offset % 100000 == 0 && tid < READ_BARRIER_NUM) {
            read_barrier_.Wait();
        }
        if (it == index_[bucket_id] + mmap_meta_cnt_[bucket_id] || it->key_ != big_endian_key_uint) {
            if (is_first_not_found) {
                log_info("not found in tid: %d\n", tid);
                is_first_not_found = false;
            }
            return kNotFound;
        }

        pread(value_file_dp_[bucket_id], value_buffer, VALUE_SIZE,
              static_cast<uint64_t>(it->value_offset_) * VALUE_SIZE);

        value->assign(value_buffer, VALUE_SIZE);
        return kSucc;
    }

    void EngineRace::ReadBucketToBuffer(uint32_t bucket_id) {
        auto range_clock_beg = high_resolution_clock::now();

        auto buffer_id = get_buffer_id(bucket_id);

        // Replace with AIO.
        uint32_t value_agg_num = 32;
        uint32_t value_num = mmap_meta_cnt_[bucket_id];
        uint32_t remain_value_num = value_num % value_agg_num;
        uint32_t total_block_num = (remain_value_num == 0 ? (value_num / value_agg_num) :
                                    (value_num / value_agg_num + 1));
        uint32_t submitted_block_num = 0;
        uint32_t completed_block_num = 0;
        uint32_t last_block_size = (remain_value_num == 0 ? (VALUE_SIZE * value_agg_num) :
                                    (remain_value_num * VALUE_SIZE));

        while (completed_block_num < total_block_num) {
            uint32_t free_nodes_num = (uint32_t) free_nodes.size();
            uint32_t remain_block_num = total_block_num - submitted_block_num;
            uint32_t to_submit = min(free_nodes_num, remain_block_num);

            if (to_submit > 0) {
                for (uint32_t i = 0; i < to_submit; ++i) {
                    uint32_t block_id = i + submitted_block_num;

                    iocb *iocb_ptr = free_nodes.front();
                    free_nodes.pop_front();

                    size_t offset = block_id * (size_t) value_agg_num * VALUE_SIZE;
                    size_t size = (block_id == (total_block_num - 1) ? last_block_size : (value_agg_num * VALUE_SIZE));
                    fill_aio_node(value_file_dp_[bucket_id], iocb_ptr,
                                  value_shared_buffers_[buffer_id] + offset, offset, size, IOCB_CMD_PREAD);

                    iocb_ptrs[i] = iocb_ptr;
                }

                auto ret = io_submit(aio_ctx, to_submit, iocb_ptrs);

                if (ret != to_submit) {
                    log_info("Commit error %d", ret);
                    exit(-1);
                }

                submitted_block_num += to_submit;
            }

            uint32_t in_flight = submitted_block_num - completed_block_num;
            uint32_t expected = (0 <= in_flight ? 0 : in_flight);

            auto ret = io_getevents(aio_ctx, expected, in_flight, io_events, NULL);

            if (ret < 0) {
                log_info("Get error.");
                exit(-1);
            }

            uint32_t to_complete = ret;
            if (to_complete > 0) {
                for (uint32_t j = 0; j < to_complete; ++j) {
                    io_event *complete_event = &io_events[j];
                    if (complete_event->res2 != 0 || complete_event->res < VALUE_SIZE) {
                        log_info("Return error.\n");
                        exit(-1);
                    }

                    iocb *iocb_ptr = (iocb *) complete_event->data;
                    free_nodes.push_back(iocb_ptr);
                }

                completed_block_num += to_complete;
            }
        }
        auto range_clock_end = high_resolution_clock::now();
        double elapsed_time = duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                              static_cast<double>(1000000000);
        total_time_ += elapsed_time;
#ifdef STAT
        if (bucket_id % 64 == 63)
            log_info("In bucket %d, Read time %.9lf s", bucket_id, elapsed_time);
#endif
    }

    void EngineRace::InitAIOContext() {
        // AIO
        // Init aio context.
        queue_depth = 32;
        aio_ctx = 0;
        iocb_ptrs = new iocb *[queue_depth];
        iocbs = new iocb[queue_depth];
        io_events = new io_event[queue_depth];
        for (uint32_t i = 0; i < queue_depth; ++i) {
            free_nodes.push_back(&iocbs[i]);
        }

        if (io_setup(queue_depth, &aio_ctx) < 0) {
            log_info("Setup fail\n");
            exit(-1);
        }
    }

    void EngineRace::InitRangeReader() {
        range_io_worker_pool_ = new ThreadPool(IO_POOL_SIZE);
        for (int i = 0; i < BUCKET_NUM; i++) {
            val_buffer_max_size_ = max<uint64_t>(val_buffer_max_size_, mmap_meta_cnt_[i]);
        }
        val_buffer_max_size_ *= VALUE_SIZE;
        log_info("Max Buffer Size: %zu B", val_buffer_max_size_);
        value_shared_buffers_ = vector<char *>(MAX_TOTAL_BUFFER_NUM);
        for (uint32_t i = 0; i < MAX_TOTAL_BUFFER_NUM; i++) {
            value_shared_buffers_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, val_buffer_max_size_);
        }
        // Value Files.
        bucket_mutex_arr_ = new mutex[BUCKET_NUM];
        bucket_cond_var_arr_ = new condition_variable[BUCKET_NUM];
        bucket_is_ready_read_ = new bool[BUCKET_NUM];
        bucket_consumed_num_ = new atomic_int[BUCKET_NUM];
        futures_.resize(BUCKET_NUM);
    }

    void EngineRace::InitForRange(int64_t tid) {
        static thread_local bool is_first = true;
        if (!is_range_init_) {
            unique_lock<mutex> lock(range_mtx_);
            if (tid == 0) {
                auto range_clock_beg = high_resolution_clock::now();
                InitRangeReader();
                InitAIOContext();
                usleep(10000);
                auto range_clock_end = high_resolution_clock::now();
                double elapsed_time = duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                                      static_cast<double>(1000000000);
                log_info("Elapsed time in first sync, %.9lf s", elapsed_time);

                // Init Barrier, Notify All.
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

        // Submit All IO Jobs.
        if (tid == 0) {
            memset(bucket_is_ready_read_, 0, BUCKET_NUM);
            for (uint32_t i = 0; i < MAX_RECYCLE_BUFFER_NUM + KEEP_REUSE_BUFFER_NUM; i++) {
                bucket_is_ready_read_[i] = true;
            }

            for (uint32_t next_bucket_idx = (is_first ? 0 : KEEP_REUSE_BUFFER_NUM);
                 next_bucket_idx < BUCKET_NUM; next_bucket_idx++) {
                bucket_consumed_num_[next_bucket_idx].store(0);
                futures_[next_bucket_idx] = range_io_worker_pool_->enqueue(
                        [this, next_bucket_idx]() {
                            unique_lock<mutex> lock(bucket_mutex_arr_[next_bucket_idx]);
                            if (!bucket_is_ready_read_[next_bucket_idx]) {
                                auto range_clock_beg = high_resolution_clock::now();

                                bucket_cond_var_arr_[next_bucket_idx].wait(lock, [this, next_bucket_idx]() {
                                    return bucket_is_ready_read_[next_bucket_idx];
                                });

                                auto range_clock_end = high_resolution_clock::now();
                                double sleep_time =
                                        duration_cast<nanoseconds>(range_clock_end - range_clock_beg).count() /
                                        static_cast<double>(1000000000);
                                total_io_sleep_time_ += sleep_time;
                            }
                            ReadBucketToBuffer(next_bucket_idx);
                            return;
                        });
            }
        }

        is_first = false;
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
#ifdef RANGE_STAT
        static thread_local uint32_t invocation_num = 0;
#endif
        static thread_local PolarString *polar_key_ptr_;
        static thread_local PolarString polar_val_ptr_;

        auto range_init_start_clock = high_resolution_clock::now();
#ifdef RANGE_STAT
        if (tid == 0) {
            log_info("Start range ts: %.9lf s in tid %d", duration_cast<nanoseconds>(
                    range_init_start_clock.time_since_epoch()).count() / 1000000000.0, tid);
        }
#endif
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
            log_info("Init elapsed time in tid %d, %.9lf s", tid, elapsed_time);
        }
        // 2-level Loop.
        uint32_t lower_key_par_id = 0;
        uint32_t upper_key_par_id = BUCKET_NUM - 1;
        for (uint32_t par_bucket_id = lower_key_par_id; par_bucket_id < upper_key_par_id + 1; par_bucket_id++) {
            range_barrier_ptr_->Wait();

            if (tid == 0) {
                auto wait_start_clock = high_resolution_clock::now();
                futures_[par_bucket_id].get();
                auto wait_end_clock = high_resolution_clock::now();
                elapsed_time = duration_cast<nanoseconds>(wait_end_clock - wait_start_clock).count() /
                               static_cast<double>(1000000000);
#ifdef STAT
                if (par_bucket_id < KEEP_REUSE_BUFFER_NUM) {
                    log_info("Elapsed wait: %.6lf s", elapsed_time);
                }
#endif
                wait_get_time_ += elapsed_time;
            } else {
                futures_[par_bucket_id].get();
            }

            uint32_t in_par_id_beg = 0;
            uint32_t in_par_id_end = mmap_meta_cnt_[par_bucket_id];
            uint64_t prev_key = 0;
#ifdef STAT
            uint32_t duplicates_num = 0;
#endif
            uint32_t buffer_id = get_buffer_id(par_bucket_id);
            for (uint32_t in_par_id = in_par_id_beg; in_par_id < in_par_id_end; in_par_id++) {
                // Skip the equalities.
                uint64_t big_endian_key = index_[par_bucket_id][in_par_id].key_;
                if (in_par_id != in_par_id_beg) {
                    if (big_endian_key == prev_key) {
#ifdef STAT
                        if (tid == 0 && duplicates_num < 3 && par_bucket_id % 256 == 0) {
                            log_info("duplicates...%d", duplicates_num);
                            duplicates_num++;
                        }
#endif
                        continue;
                    }
                }
                prev_key = big_endian_key;

                // Key (to little endian first).
                (*(uint64_t *) polar_key_ptr_->data()) = bswap_64(big_endian_key);

                // Value.
                uint64_t val_id = index_[par_bucket_id][in_par_id].value_offset_;
                polar_val_ptr_ = PolarString(value_shared_buffers_[buffer_id] + val_id * VALUE_SIZE,
                                             VALUE_SIZE);
                // Visit Key/Value.
                visitor.Visit(*polar_key_ptr_, polar_val_ptr_);
            }
            // End of inner loop, Submit IO Jobs.
            int32_t my_order = ++bucket_consumed_num_[par_bucket_id];
            if (my_order == total_range_num_threads_) {
                uint32_t next_bucket_idx = par_bucket_id + MAX_RECYCLE_BUFFER_NUM;

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
        // Flush Values.
        bool is_flush_key = false;
        bool is_flush_val = false;
        for (int i = 0; i < BUCKET_NUM; i++) {
            const string value_file_path = dir + "/" + value_file_name;
            string temp_value = value_file_path + to_string(i);
            const string tmp_value_file_path = dir + "/polar.valbuffers";
            string temp_buffer_value = tmp_value_file_path + to_string(i);

            if ((mmap_meta_cnt_[i] % TMP_VALUE_BUFFER_SIZE) != 0 && file_size(temp_buffer_value.c_str()) > 0) {
                if (!is_flush_val) {
                    log_info("Flush Val in Bucket %d", i);
                    is_flush_val = true;
                }
                value_buffer_file_dp_[i] = open(temp_buffer_value.c_str(), O_RDWR, FILE_PRIVILEGE);
                size_t tmp_buffer_value_file_size = VALUE_SIZE * TMP_VALUE_BUFFER_SIZE;
                mmap_value_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_value_file_size,
                                                              PROT_READ | PROT_WRITE, MAP_SHARED,
                                                              value_buffer_file_dp_[i], 0);

                size_t write_length = (mmap_meta_cnt_[i] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                size_t write_offset = static_cast<uint64_t>(mmap_meta_cnt_[i] / TMP_VALUE_BUFFER_SIZE *
                                                            TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                auto tmp_fd = open(temp_value.c_str(), O_RDWR, FILE_PRIVILEGE);
                pwrite(tmp_fd, mmap_value_aligned_buffer_[i], write_length, write_offset);
                close(tmp_fd);
                ftruncate(value_buffer_file_dp_[i], 0);
            }
        }
        // Flush Keys.
        for (int i = 0; i < BUCKET_NUM; i++) {
            const string key_file_path = dir + "/" + key_file_name;
            string temp_key = key_file_path + to_string(i);
            const string tmp_key_file_path = dir + "/polar.keybuffers";
            string temp_buffer_key = tmp_key_file_path + to_string(i);

            if ((mmap_meta_cnt_[i] % TMP_KEY_BUFFER_SIZE) != 0 && file_size(temp_buffer_key.c_str()) > 0) {
                if (!is_flush_key) {
                    log_info("Flush Key in Bucket %d", i);
                    is_flush_key = true;
                }
                key_buffer_file_dp_[i] = open(temp_buffer_key.c_str(), O_RDWR, FILE_PRIVILEGE);
                constexpr size_t tmp_buffer_key_file_size = sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE;
                mmap_key_aligned_buffer_[i] = (uint64_t *) mmap(nullptr, tmp_buffer_key_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, key_buffer_file_dp_[i], 0);

                size_t write_length = (mmap_meta_cnt_[i] % TMP_KEY_BUFFER_SIZE) * sizeof(uint64_t);
                size_t write_offset = static_cast<uint64_t>(mmap_meta_cnt_[i] / TMP_KEY_BUFFER_SIZE *
                                                            TMP_KEY_BUFFER_SIZE) * sizeof(uint64_t);
                auto tmp_fd = open(temp_key.c_str(), O_RDWR, FILE_PRIVILEGE);
                pwrite(tmp_fd, mmap_key_aligned_buffer_[i], write_length, write_offset);
                close(tmp_fd);
                ftruncate(key_buffer_file_dp_[i], 0);
            }
        }

        clock_end = high_resolution_clock::now();
        log_info("After Flush Files, time: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0);
    }

    void EngineRace::BuildIndex() {
        clock_end = high_resolution_clock::now();
        log_info("Begin BI, time: %.3lf s", duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0);
        // Key Cnt, Index Allocation.
        index_ = vector<KeyEntry *>(BUCKET_NUM, nullptr);
        for (int key_par_id = 0; key_par_id < BUCKET_NUM; key_par_id++) {
            index_[key_par_id] = static_cast<KeyEntry *>(malloc(mmap_meta_cnt_[key_par_id] * sizeof(KeyEntry)));
        }

        // Read each key file.
        auto **local_buffers_g = new uint64_t *[NUM_READ_KEY_THREADS];
        for (uint32_t tid = 0; tid < NUM_READ_KEY_THREADS; ++tid) {
            local_buffers_g[tid] = (uint64_t *) memalign(FILESYSTEM_BLOCK_SIZE,
                                                         sizeof(uint64_t) * KEY_READ_BLOCK_COUNT);
        }
        vector<thread> workers(NUM_READ_KEY_THREADS);
        for (uint32_t tid = 0; tid < NUM_READ_KEY_THREADS; ++tid) {
            workers[tid] = thread([tid, local_buffers_g, this]() {
                uint64_t *local_buffer = local_buffers_g[tid];
                for (uint32_t key_par_id = tid; key_par_id < BUCKET_NUM; key_par_id += NUM_READ_KEY_THREADS) {
                    uint32_t entry_count = mmap_meta_cnt_[key_par_id];
                    if (entry_count > 0) {
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
                            if (ret < static_cast<ssize_t>(remain_entries_count * sizeof(uint64_t))) {
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
            });
        }
        for (uint32_t i = 0; i < NUM_READ_KEY_THREADS; ++i) {
            workers[i].join();
            free(local_buffers_g[i]);
        }
        delete[]local_buffers_g;
        clock_end = high_resolution_clock::now();
        log_info("Finish BI, time: %.3lf s", duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0);
    }

}  // namespace polar_race
