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

    using namespace std;

    atomic_int write_num_threads(-1);
    atomic_int read_num_threads_count(-1);

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
            barrier_(WRITE_BARRIER_NUM), read_barrier_(READ_BARRIER_NUM), is_sorted_(nullptr) {
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
                ftruncate(write_value_file_dp_[i], static_cast<uint64_t>(VALUE_SIZE) * (TOTAL_COUNT / VAL_BUCKET_NUM));

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
                          static_cast<uint64_t>(sizeof(KeyEntry)) * (TOTAL_COUNT / VAL_BUCKET_NUM));

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

        sleep(20);
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
        if (!is_sorted_[key_par_id]) {
            unique_lock<mutex> lock(key_mtx_[key_par_id]);
            if (!is_sorted_[key_par_id]) {
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
        pread(write_value_file_dp_[val_par_id], value_buffer, VALUE_SIZE, (uint64_t) (it->value_offset_) * VALUE_SIZE);

        value->assign(value_buffer, VALUE_SIZE);
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
        // Read each key file.
        vector <thread> workers(NUM_THREADS);
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
//                        sort(index_[key_par_id], index_[key_par_id] + mmap_key_meta_cnt_[key_par_id],
//                             [](KeyEntry l, KeyEntry r) {
//                                 if (l.key_ == r.key_) {
//                                     return l.value_offset_ > r.value_offset_;
//                                 } else {
//                                     return l.key_ < r.key_;
//                                 }
//                             });
                    }
                }
            }));
        }
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            workers[i].join();
        }
        clock_end = high_resolution_clock::now();
        log_info("Finish BI, time: %.3lf s, ts: %.3lf s",
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
    }
}  // namespace polar_race
