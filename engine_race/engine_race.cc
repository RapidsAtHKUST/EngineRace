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
#include <aio.h>
#include <linux/aio_abi.h>
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

    std::string exec(const char *cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
        if (!pipe) throw std::runtime_error("popen() failed!");
        while (!feof(pipe.get())) {
            if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
                result += buffer.data();
        }
        return result;
    }

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

    inline void setThreadSelfAffinity(int core_id) {
//        long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
//        assert(core_id >= 0 && core_id < num_cores);
//        if (core_id == 0) {
//            printf("affinity relevant logical cores: %ld\n", num_cores);
//        }
//        core_id = core_id % num_cores;
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);

        pthread_t current_thread = pthread_self();
        pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
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
    const string value_buffer_file_name = "polar.valbuffers";
    const string key_buffer_file_name = "polar.keybuffers";

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
            write_key_file_dp_(nullptr), write_value_file_dp_(nullptr), write_value_buffer_file_dp_(nullptr),
            write_key_buffer_file_dp_(nullptr), write_meta_file_dp_(-1), write_mmap_meta_file_(nullptr),
            mmap_value_aligned_buffer_(nullptr), mmap_key_aligned_buffer_(nullptr), aligned_buffer_(nullptr),
            tmp_value_buf_size_(NUM_THREADS, 4), lower_bound_cost_(NUM_THREADS, 0), dir_(dir) {
        clock_end = high_resolution_clock::now();
        log_info("Start init DB, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        const string meta_file_path = dir + "/" + meta_file_name;
        const string key_file_path = dir + "/" + key_file_name;
        const string value_file_path = dir + "/" + value_file_name;
        const string tmp_value_file_path = dir + "/" + value_buffer_file_name;
        const string tmp_key_file_path = dir + "/" + key_buffer_file_name;

        const size_t key_file_size = sizeof(uint64_t) * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;
        const size_t value_file_size = VALUE_SIZE * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;


//        for (int i = 0; i < NUM_THREADS; i++) {
//            if (i % 4 == 0) {
//                tmp_value_buf_size_[i] = 4;
//            } else if (i % 4 == 1) {
//                tmp_value_buf_size_[i] = 4;
//            } else if (i % 4 == 2) {
//                tmp_value_buf_size_[i] = 4;
//            }
//        }
//        const size_t tmp_buffer_value_file_size = VALUE_SIZE * (size_t) TMP_VALUE_BUFFER_SIZE;
        const size_t tmp_buffer_key_file_size = sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE;

        write_key_file_dp_ = new int[NUM_THREADS];
        write_value_file_dp_ = new int[NUM_THREADS];
        write_value_buffer_file_dp_ = new int[NUM_THREADS];
        write_key_buffer_file_dp_ = new int[NUM_THREADS];

        mmap_value_aligned_buffer_ = new char *[NUM_THREADS];
        mmap_key_aligned_buffer_ = new uint64_t *[NUM_THREADS];

        if (!file_exists(meta_file_path.c_str())) {
            log_info("Initialize the database...");

            write_meta_file_dp_ = open(meta_file_path.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);

            if (write_meta_file_dp_ < 0) {
                log_info("Fail to create the meta file.");
                exit(-1);
            }
            ftruncate(write_meta_file_dp_, VALUE_SIZE * NUM_THREADS);

            write_mmap_meta_file_ = new uint32_t *[NUM_THREADS];
            aligned_buffer_ = new char *[NUM_THREADS];

            vector<thread> workers(NUM_THREADS);
            for (int i = 0; i < NUM_THREADS; ++i) {
                workers[i] = move(thread([&key_file_path, &value_file_path, &tmp_value_file_path, & tmp_key_file_path,
                                                 i, this]() {
                    string temp_key = key_file_path + to_string(i);
                    string temp_value = value_file_path + to_string(i);
                    string temp_buffer_value = tmp_value_file_path + to_string(i);
                    string temp_buffer_key = tmp_key_file_path + to_string(i);

                    write_key_file_dp_[i] = open(temp_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                    write_value_file_dp_[i] = open(temp_value.c_str(), O_RDWR | O_CREAT | O_DIRECT, FILE_PRIVILEGE);
                    write_value_buffer_file_dp_[i] = open(temp_buffer_value.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);
                    write_key_buffer_file_dp_[i] = open(temp_buffer_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);

                    if (write_key_file_dp_[i] < 0 || write_value_file_dp_[i] < 0 ||
                        write_value_buffer_file_dp_[i] < 0) {
                        log_info("Fail to create key-value files.");
                        exit(-1);
                    }
                    // Pre-allocate on the SSD.
//                    fallocate(write_key_file_dp_[i], 0, 0, key_file_size);
//                    fallocate(write_value_file_dp_[i], 0, 0, value_file_size);
//                    fallocate(write_value_buffer_file_dp_[i], 0, 0, tmp_buffer_value_file_size);
//                    fallocate(write_key_buffer_file_dp_[i], 0, 0, tmp_buffer_key_file_size);
                    ftruncate(write_key_file_dp_[i], key_file_size);
                    ftruncate(write_value_file_dp_[i], value_file_size);
                    size_t tmp_buffer_value_file_size = VALUE_SIZE * tmp_value_buf_size_[i];
                    ftruncate(write_value_buffer_file_dp_[i], tmp_buffer_value_file_size);
                    ftruncate(write_key_buffer_file_dp_[i], tmp_buffer_key_file_size);

                    mmap_value_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_value_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, write_value_buffer_file_dp_[i], 0);
                    mmap_key_aligned_buffer_[i] = (uint64_t *) mmap(nullptr, tmp_buffer_key_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, write_key_buffer_file_dp_[i], 0);

                    write_mmap_meta_file_[i] = (uint32_t *) mmap(nullptr, VALUE_SIZE, PROT_READ | PROT_WRITE,
                                                                 MAP_SHARED,
                                                                 write_meta_file_dp_, i * VALUE_SIZE);

                    memset(write_mmap_meta_file_[i], 0, sizeof(uint32_t) * (NUM_THREADS + 1));
                    aligned_buffer_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, VALUE_SIZE);
                }));
            }
            for (int i = 0; i < NUM_THREADS; i++) {
                workers[i].join();
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
                string temp_buffer_value = tmp_value_file_path + to_string(i);
                string temp_buffer_key = tmp_key_file_path + to_string(i);

                write_key_file_dp_[i] = open(temp_key.c_str(), O_RDONLY, FILE_PRIVILEGE);
                write_value_file_dp_[i] = open(temp_value.c_str(), O_RDONLY | O_DIRECT, FILE_PRIVILEGE);
                write_value_buffer_file_dp_[i] = open(temp_buffer_value.c_str(), O_RDWR, FILE_PRIVILEGE);
                write_key_buffer_file_dp_[i] = open(temp_buffer_key.c_str(), O_RDWR | O_CREAT, FILE_PRIVILEGE);

                if (write_key_file_dp_[i] < 0 || write_value_file_dp_[i] < 0 || write_value_buffer_file_dp_[i] < 0) {
                    log_info("Fail to open key-value files.");
                    exit(-1);
                }
                size_t tmp_buffer_value_file_size = VALUE_SIZE * tmp_value_buf_size_[i];

                mmap_value_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_value_file_size,
                                                              PROT_READ | PROT_WRITE, MAP_SHARED,
                                                              write_value_buffer_file_dp_[i], 0);
                mmap_key_aligned_buffer_[i] = (uint64_t *) mmap(nullptr, tmp_buffer_key_file_size, \
                        PROT_READ | PROT_WRITE, MAP_SHARED, write_key_buffer_file_dp_[i], 0);

                aligned_buffer_[i] = (char *) memalign(FILESYSTEM_BLOCK_SIZE, VALUE_SIZE);
            }
            log_info("Open the files.");

            BuildIndex(dir);

            log_info("Reload the database successfully.");
        }
        clock_end = high_resolution_clock::now();
        log_info("After init DB, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
    }

// 1. Open engine
    RetCode EngineRace::Open(const std::string &name, Engine **eptr) {
        log_info("hostname: %s", exec("hostname").c_str());
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
            if (write_mmap_meta_file_ != nullptr) {
                munmap(write_mmap_meta_file_[i], VALUE_SIZE);
            }
            if (mmap_value_aligned_buffer_[i] != nullptr) {
                size_t TMP_VALUE_BUFFER_SIZE = tmp_value_buf_size_[i];
                munmap(mmap_value_aligned_buffer_[i], VALUE_SIZE * (size_t) TMP_VALUE_BUFFER_SIZE);
            }
            if (mmap_key_aligned_buffer_[i] != nullptr) {
                munmap(mmap_key_aligned_buffer_[i], sizeof(uint64_t) * (size_t) TMP_KEY_BUFFER_SIZE);
            }
            if (aligned_buffer_ != nullptr) {
                free(aligned_buffer_[i]);
            }
            close(write_key_file_dp_[i]);
            close(write_value_file_dp_[i]);
            close(write_value_buffer_file_dp_[i]);
            close(write_key_buffer_file_dp_[i]);
        }

        delete[] write_key_file_dp_;
        delete[] write_value_file_dp_;
        delete[] write_mmap_meta_file_;
        delete[] aligned_buffer_;
        delete[] mmap_value_aligned_buffer_;
        delete[] mmap_key_aligned_buffer_;
        for (KeyEntry *index_partition: index_) {
            free(index_partition);
        }
//        if (total_cnt_.size() > 0) {
//            for (auto i = 0; i < NUM_THREADS; i++) {
//                log_info("time for bs: %lld ns", lower_bound_cost_[i]);
//            }
//        }

        clock_end = high_resolution_clock::now();
        log_info("Finish ~EngineRace(), mem usage: %s KB, time: %.3lf s, ts: %.3lf s",
                 FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);

        if (total_cnt_.size() > 0 && (total_cnt_[0] > 500000)) {
            log_info("Close.");
            Benchmark();
        }
    }

// 3. Write a key-value pair into engine
    RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
        static thread_local uint32_t tid = (uint32_t) (++write_num_threads) % NUM_THREADS;
        static thread_local uint32_t *mmap_local_meta_file_ = write_mmap_meta_file_[tid];
        static thread_local int local_key_file = write_key_file_dp_[tid];
        static thread_local int local_value_file = write_value_file_dp_[tid];
        static thread_local uint32_t local_block_offset = 0;
        static thread_local char *value_buffer = mmap_value_aligned_buffer_[tid];
        static thread_local uint64_t *key_buffer = mmap_key_aligned_buffer_[tid];
        static thread_local uint32_t TMP_VALUE_BUFFER_SIZE = tmp_value_buf_size_[tid];
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> first_write_clk;
        static thread_local std::chrono::time_point<std::chrono::high_resolution_clock> last_write_clk;
        if (local_block_offset == 0) {
            first_write_clk = high_resolution_clock::now();
        }
#ifdef AFFINITY
        if (local_block_offset == 0) {
            setThreadSelfAffinity(tid);
        }
#endif
#ifdef DEBUG
        if (tid == 0 && local_block_offset == 0) {
            log_info("First Write, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                     duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                     std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() /
                     1000.0);
        }
#endif
        // Write value to the value file, with a tmp file as value_buffer.
        uint32_t val_buffer_offset = (local_block_offset % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
        memcpy(value_buffer + val_buffer_offset, value.data(), VALUE_SIZE);
        if (((local_block_offset + 1) % TMP_VALUE_BUFFER_SIZE) == 0) {
            pwrite(local_value_file, value_buffer, VALUE_SIZE * TMP_VALUE_BUFFER_SIZE,
                   ((uint64_t) local_block_offset - (TMP_VALUE_BUFFER_SIZE - 1)) * VALUE_SIZE);
        }

        // Write key to the key file.
        uint64_t key_int = TO_UINT64(key.data());
        uint32_t key_buffer_offset = (local_block_offset % TMP_KEY_BUFFER_SIZE);
        key_buffer[key_buffer_offset] = key_int;
        if (((local_block_offset + 1) % TMP_KEY_BUFFER_SIZE) == 0) {
            pwrite(local_key_file, key_buffer, sizeof(uint64_t) * TMP_KEY_BUFFER_SIZE,
                   ((uint64_t) local_block_offset - (TMP_KEY_BUFFER_SIZE - 1)) * sizeof(uint64_t));
        }

        // Update the meta data.
        local_block_offset += 1;
        if (local_block_offset == 1000000) {
            last_write_clk = high_resolution_clock::now();
            log_info("Write Stat of tid %d, mem usage: %s KB, elapsed time: %.3lf s, ts: %.3lf s",
                     tid, FormatWithCommas(getValue()).c_str(),
                     duration_cast<milliseconds>(last_write_clk - first_write_clk).count() / 1000.0,
                     duration_cast<milliseconds>(last_write_clk.time_since_epoch()).count() / 1000.0);
        }
        mmap_local_meta_file_[0] = local_block_offset;
        mmap_local_meta_file_[get_partition_id(key_int) + 1]++;
        return kSucc;
    }

// 4. Read value of a key
    RetCode EngineRace::Read(const PolarString &key, std::string *value) {
        static thread_local int64_t tid = (++read_num_threads_count) % NUM_THREADS;
        static thread_local char *value_buffer = aligned_buffer_[tid];
        static thread_local bool is_first_not_found = true;

        uint64_t key_uint = TO_UINT64(key.data());

        KeyEntry tmp{};
        tmp.key_ = key_uint;
        auto partition_id = get_partition_id(key_uint);

//        auto clk_beg = high_resolution_clock::now();

        auto it = index_[partition_id] + branchfree_search(index_[partition_id], total_cnt_[partition_id], tmp);
//        auto clk_end = high_resolution_clock::now();
//        lower_bound_cost_[tid] += duration_cast<nanoseconds>(clk_end - clk_beg).count();

        if (it == index_[partition_id] + total_cnt_[partition_id] || it->key_ != key_uint) {
            if (is_first_not_found) {
                log_info("not found in tid: %d\n", tid);
                is_first_not_found = false;
            }
            return kNotFound;
        }

        ValueOffset value_offset = it->value_offset_;
        pread(write_value_file_dp_[value_offset.partition_], value_buffer, VALUE_SIZE,
              (uint64_t) (value_offset.block_offset_) * VALUE_SIZE);

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

    void EngineRace::BuildIndex(string dir) {
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
            }

            // Flush tmp files.
            const string value_file_path = dir + "/" + value_file_name;
            string temp_value = value_file_path + to_string(tid);
            size_t TMP_VALUE_BUFFER_SIZE = tmp_value_buf_size_[tid];
            if ((entry_counts[tid] % TMP_VALUE_BUFFER_SIZE) != 0) {
                size_t write_length = (entry_counts[tid] % TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                size_t write_offset = static_cast<uint64_t>(entry_counts[tid] / TMP_VALUE_BUFFER_SIZE *
                                                            TMP_VALUE_BUFFER_SIZE) * VALUE_SIZE;
                auto tmp_fd = open(temp_value.c_str(), O_RDWR, FILE_PRIVILEGE);
                pwrite(tmp_fd, mmap_value_aligned_buffer_[tid], write_length, write_offset);
                close(tmp_fd);
            }
            const string key_file_path = dir + "/" + key_file_name;
            string temp_key = key_file_path + to_string(tid);
            if ((entry_counts[tid] % TMP_KEY_BUFFER_SIZE) != 0) {
                size_t write_length = (entry_counts[tid] % TMP_KEY_BUFFER_SIZE) * sizeof(uint64_t);
                size_t write_offset = static_cast<uint64_t>(entry_counts[tid] / TMP_KEY_BUFFER_SIZE *
                                                            TMP_KEY_BUFFER_SIZE) * sizeof(uint64_t);
                auto tmp_fd = open(temp_key.c_str(), O_RDWR, FILE_PRIVILEGE);
                pwrite(tmp_fd, mmap_key_aligned_buffer_[tid], write_length, write_offset);
                close(tmp_fd);
            }
        }
        log_info("Init Ready, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
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
                auto *buffer = (uint64_t *) malloc(sizeof(uint64_t) * KEY_READ_BLOCK_COUNT);
//                posix_fadvise(write_key_file_dp_[tid], 0, entry_counts[tid] * sizeof(uint64_t), POSIX_FADV_SEQUENTIAL);
                readahead(write_key_file_dp_[tid], 0, entry_counts[tid] * sizeof(uint64_t));
                vector<uint32_t> par_off(NUM_THREADS);
                for (size_t j = 0; j < par_off.size(); j++) {
                    par_off[j] = par_prefix_sum_arr[tid][j];
                }
                uint32_t entry_count = entry_counts[tid];
                uint32_t passes = entry_count / KEY_READ_BLOCK_COUNT;
                uint32_t remain_entries_count = entry_count - passes * KEY_READ_BLOCK_COUNT;
                size_t read_offset = 0;
                uint32_t file_offset = 0;
                for (uint32_t j = 0; j < passes; ++j) {
                    pread(write_key_file_dp_[tid], buffer, KEY_READ_BLOCK_COUNT * sizeof(uint64_t), read_offset);

                    for (int k = 0; k < KEY_READ_BLOCK_COUNT; k++) {
                        auto par_id = get_partition_id(buffer[k]);
                        index_[par_id][par_off[par_id]].key_ = buffer[k];
                        index_[par_id][par_off[par_id]].value_offset_.block_offset_ = file_offset;
                        index_[par_id][par_off[par_id]].value_offset_.partition_ = tid;
                        file_offset++;
                        par_off[par_id]++;
                    }
                    read_offset += KEY_READ_BLOCK_COUNT * sizeof(uint64_t);
                }

                if (remain_entries_count != 0) {
                    pread(write_key_file_dp_[tid], buffer, remain_entries_count * sizeof(uint64_t), read_offset);
                    for (uint32_t k = 0; k < remain_entries_count; k++) {
                        auto par_id = get_partition_id(buffer[k]);
                        index_[par_id][par_off[par_id]].key_ = buffer[k];
                        index_[par_id][par_off[par_id]].value_offset_.block_offset_ = file_offset;
                        index_[par_id][par_off[par_id]].value_offset_.partition_ = tid;
                        file_offset++;
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

        for (uint32_t tid = 0; tid < NUM_THREADS; ++tid) {
            workers[tid] = move(thread([tid, this]() {
                sort(index_[tid], index_[tid] + total_cnt_[tid], [](KeyEntry l, KeyEntry r) {
                    if (l.key_ == r.key_) {
                        return l.value_offset_.block_offset_ > r.value_offset_.block_offset_;
                    } else {
                        return l.key_ < r.key_;
                    }
                });
            }));
        }
        for (uint32_t i = 0; i < NUM_THREADS; ++i) {
            workers[i].join();
        }
        clock_end = high_resolution_clock::now();

        log_info("Finish BI, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
    }

    void EngineRace::TestDevice(int open_write_file_flag, uint32_t *write_file_block_offset, uint32_t write_block_num,
                                int open_read_file_flag, uint32_t *read_file_block_offset, uint32_t read_block_num,
                                uint32_t thread_num, uint32_t block_size, uint32_t alignment_size) {
        const string value_file_path = dir_ + "/" + value_file_name;
        const string key_file_path = dir_ + "/" + key_file_name;

        write_value_file_dp_ = new int[thread_num];
        write_key_file_dp_ = new int[thread_num];
        aligned_buffer_ = new char*[thread_num];
        string temp_dir = "rm -r " + dir_ +  "/*";
        log_info("%s", temp_dir.c_str());
        exec(temp_dir.c_str());

        auto start = high_resolution_clock::now();
        for (uint32_t i = 0; i < thread_num; ++i) {
            string temp_value_file = value_file_path + to_string(i);
            string temp_key_file = key_file_path + to_string(i);

            write_value_file_dp_[i] = open(temp_value_file.c_str(), open_write_file_flag, FILE_PRIVILEGE);
            write_key_file_dp_[i] = open(temp_key_file.c_str(), O_CREAT | O_RDWR, FILE_PRIVILEGE);

            if (write_value_file_dp_[i] < 0 || write_key_file_dp_[i] < 0) {
                log_info("Fail to open key-value files.");
                exit(-1);
            }

            fallocate(write_value_file_dp_[i], 0, 0, ((size_t)FILESYSTEM_BLOCK_SIZE) * KEY_VALUE_MAX_COUNT_PER_THREAD);
            aligned_buffer_[i] = (char *) memalign(alignment_size, block_size);
        }

        auto end = high_resolution_clock::now();
        log_info("Release %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);
        vector<thread> workers(thread_num);

        start = high_resolution_clock::now();

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i] = move(thread([write_file_block_offset, block_size, write_block_num, i, this]() {
                int local_value_file_dp = write_value_file_dp_[i];

                char* value_buffer = aligned_buffer_[i];
                char local_value[VALUE_SIZE];

                for (uint32_t j = 0; j < write_block_num; ++j) {
                    size_t write_value_offset = (size_t)write_file_block_offset[j] * block_size;
//                    size_t write_key_offset = (size_t)write_file_block_offset[j] * sizeof(KeyEntry);
//                    local_value[0] = 10;
//                    memcpy(value_buffer, local_value, VALUE_SIZE);
//                    local_value[8] = 20;
//                    memcpy(value_buffer, local_value, VALUE_SIZE);
//                    local_value[16] = 30;
                    memcpy(value_buffer, local_value, VALUE_SIZE);
                    pwrite(local_value_file_dp, value_buffer, block_size, write_value_offset);

//                    KeyEntry key_entry;
//                    key_entry.key_ = j;
//                    key_entry.value_offset_.block_offset_ = i;
//                    key_entry.value_offset_.block_offset_ = write_file_block_offset[j];
//
//                    pwrite(local_key_file_dp, &key_entry, sizeof(KeyEntry), write_key_offset);
                }
            }));
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i].join();
        }

        end = high_resolution_clock::now();
        log_info("Step one %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);

        for (uint32_t i = 0; i < thread_num; ++i) {
            fsync(write_value_file_dp_[i]);
            close(write_value_file_dp_[i]);
            fsync(write_key_file_dp_[i]);
            close(write_key_file_dp_[i]);
        }
        delete[] write_key_file_dp_;

        end = high_resolution_clock::now();
        log_info("Step one %s, %s, %.3lf end.", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);
//        for (uint32_t i = 0; i < thread_num; ++i) {
//            string temp_value = value_file_path + to_string(i);
//
//            write_value_file_dp_[i] = open(temp_value.c_str(), open_read_file_flag, FILE_PRIVILEGE);
//
//            if (write_value_file_dp_[i] < 0) {
//                log_info("Fail to open key-value files.");
//                exit(-1);
//            }
//        }
//
//        start = high_resolution_clock::now();
//
//        for (uint32_t i = 0; i < thread_num; ++i) {
//            workers[i] = move(thread([read_file_block_offset, block_size, read_block_num, i, this]() {
//                int local_file_dp = write_value_file_dp_[i];
//                char* value_buffer = aligned_buffer_[i];
//
//                for (uint32_t j = 0; j < read_block_num; ++j) {
//                    size_t read_offset = (size_t)read_file_block_offset[j] * block_size;
//                    pread(local_file_dp, value_buffer, block_size, read_offset);
//                }
//            }));
//        }
//
//        for (uint32_t i = 0; i < thread_num; ++i) {
//            workers[i].join();
//        }
//
//        end = high_resolution_clock::now();
//        log_info("Step two %s, %s, %.3lf", strerror(errno),
//                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);
//
//        for (uint32_t i = 0; i < thread_num; ++i) {
//            fsync(write_value_file_dp_[i]);
//            close(write_value_file_dp_[i]);
//            free(aligned_buffer_[i]);
//        }

        delete[] aligned_buffer_;
        delete[] write_value_file_dp_;
    }

    void EngineRace::Benchmark() {
        const size_t value_file_size = (size_t) VALUE_SIZE * KEY_VALUE_MAX_COUNT_PER_THREAD * 32;
        vector<uint32_t> block_size_config = {4096};
        vector<uint32_t> alignment_size_config = {4096};
        vector<uint32_t> thread_num_config = {1};
        vector<uint32_t> ring_size_config = {32, 64};
        uint32_t flag_config_num = 1;
        vector<int> write_file_flags_config = {O_CREAT | O_RDWR | O_DIRECT};
        vector<int> read_file_flags_config = {O_CREAT | O_RDONLY | O_DIRECT};

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
                        for (uint32_t ring_size : ring_size_config) {
                            log_info("%d", count++);
                            TestDevice(write_file_flags, file_block_offset, block_num, thread_num, block_size,
                                       alignment_size, ring_size);
                        }
                    }
                }
            }
            delete[] file_block_offset;
        }

        log_info("Close file end..");
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

    void EngineRace::TestDevice(int open_write_file_flag, uint32_t *write_file_block_offset, uint32_t write_block_num,
                                uint32_t thread_num, uint32_t block_size, uint32_t alignment_size, uint32_t ring_size) {
        const string value_file_path = dir_ + "/" + value_file_name;
        const string key_file_path = dir_ + "/" + key_file_name;

        write_value_file_dp_ = new int[thread_num];
        char** ring_value_buffer = new char*[thread_num];
        aiocb** ring_aiocb_buffer = new aiocb*[thread_num];

        string temp_dir = "rm -r " + dir_ +  "/*";
        log_info("%s", temp_dir.c_str());
        exec(temp_dir.c_str());

        auto start = high_resolution_clock::now();
        for (uint32_t i = 0; i < thread_num; ++i) {
            string temp_value_file = value_file_path + to_string(i);

            write_value_file_dp_[i] = open(temp_value_file.c_str(), open_write_file_flag, FILE_PRIVILEGE);

            if (write_value_file_dp_[i] < 0) {
                log_info("Fail to open key-value files.");
                exit(-1);
            }

            fallocate(write_value_file_dp_[i], 0, 0, ((size_t)FILESYSTEM_BLOCK_SIZE) * KEY_VALUE_MAX_COUNT_PER_THREAD * 32);

            ring_value_buffer[i] = (char*) memalign(alignment_size, ring_size * block_size);
            ring_aiocb_buffer[i] = new aiocb[ring_size];
            memset(ring_aiocb_buffer[i], 0, sizeof(aiocb) * ring_size);
        }

        auto end = high_resolution_clock::now();
        log_info("Release %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);
        vector<thread> workers(thread_num);

        start = high_resolution_clock::now();

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i] = move(thread([write_file_block_offset, block_size, write_block_num, i, ring_aiocb_buffer, ring_value_buffer, ring_size, this]() {
                int local_value_file_dp = write_value_file_dp_[i];
                char* local_ring_value_buffer = ring_value_buffer[i];
                aiocb* local_ring_aiocb_buffer = ring_aiocb_buffer[i];
                uint32_t pending_cnt = 0;
                uint32_t next_id = 0;
                char local_value[VALUE_SIZE];

                for (uint32_t j = 0; j < write_block_num; ++j) {
                    if (pending_cnt >= ring_size) {
                        int err;
                        while ((err = aio_error(&local_ring_aiocb_buffer[next_id])) == EINPROGRESS);
                        pending_cnt -= 1;

                        ssize_t ret = aio_return(&local_ring_aiocb_buffer[next_id]);
                        if (err != 0) {
                            log_info("Error %d", err);
                            return;
                        }
                        if (ret != block_size) {
                            log_info("Return %zu", ret);
                            return;
                        }
                    }

                    size_t write_value_offset = (size_t)write_file_block_offset[j] * block_size;
                    local_value[0] = 'a';
                    memcpy(local_ring_value_buffer + (size_t)block_size * next_id, local_value, VALUE_SIZE);

                    local_ring_aiocb_buffer[next_id].aio_fildes = local_value_file_dp;
                    local_ring_aiocb_buffer[next_id].aio_offset = write_value_offset;
                    local_ring_aiocb_buffer[next_id].aio_nbytes = block_size;
                    local_ring_aiocb_buffer[next_id].aio_lio_opcode = IOCB_CMD_PWRITE;
                    local_ring_aiocb_buffer[next_id].aio_buf = local_ring_value_buffer + (size_t)block_size * next_id;

                    if (aio_write(&local_ring_aiocb_buffer[next_id]) < 0) {
                        log_info("Error write");
                        return;
                    }

                    next_id += 1;
                    if (next_id == ring_size) {
                        next_id = 0;
                    }

                    pending_cnt += 1;
                }
            }));
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i].join();
        }

        end = high_resolution_clock::now();
        log_info("Step one %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);

        for (uint32_t i = 0; i < thread_num; ++i) {
            fsync(write_value_file_dp_[i]);
            close(write_value_file_dp_[i]);
            delete ring_value_buffer[i];
            delete ring_aiocb_buffer[i];
        }
        delete[] write_value_file_dp_;
        delete[] ring_value_buffer;
        delete[] ring_aiocb_buffer;

        end = high_resolution_clock::now();
        log_info("Step one %s, %s, %.3lf end.", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);
    }
}  // namespace polar_race
