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
#include <fcntl.h>
#include <sys/syscall.h>
#include <list>
#include "log.h"

#include "util.h"
#include "stat.h"

//#include <tbb/parallel_sort.h>
//#include <parallel/algorithm>
//#include "parasort.h"

namespace polar_race {
    using namespace std::chrono;

    struct AioNode {
        char* value_buffer_ptr_;
        iocb* iocb_ptr_;
    };

    static inline long
    io_setup(unsigned maxevents, aio_context_t *ctx) {
        return syscall(SYS_io_setup, maxevents, ctx);
    }

    static inline long
    io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) {
        return syscall(SYS_io_submit, ctx, nr, iocbpp);
    }

    static inline long
    io_getevents(aio_context_t ctx, long min_nr, long nr,
                 struct io_event *events, struct timespec *timeout) {
        return syscall(SYS_io_getevents, ctx, min_nr, nr, events, timeout);
    }

    static inline long
    io_destroy(aio_context_t ctx) {
        return syscall(SYS_io_destroy, ctx);
    }

    static inline void
    fill_aio_node(int fd, AioNode* aio_node, size_t offset, size_t buffer_size, uint16_t operation) {
        memset(aio_node->iocb_ptr_, 0, sizeof(iocb));
        aio_node->iocb_ptr_->aio_buf = (uintptr_t) aio_node->value_buffer_ptr_;
        aio_node->iocb_ptr_->aio_data = (uintptr_t) aio_node;
        aio_node->iocb_ptr_->aio_fildes = fd;
        aio_node->iocb_ptr_->aio_lio_opcode = operation;
        aio_node->iocb_ptr_->aio_reqprio = 0;
        aio_node->iocb_ptr_->aio_nbytes = buffer_size;
        aio_node->iocb_ptr_->aio_offset = offset;
    }

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
            log_info("Close.......");
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
                                uint32_t thread_num, uint32_t block_size, uint32_t alignment_size, uint32_t queue_depth) {
        const string value_file_path = dir_ + "/" + value_file_name;

        // Remove existing files in the dir_.
        string temp_dir = "rm -r " + dir_ +  "/*";
        exec(temp_dir.c_str());

        write_value_file_dp_ = new int[thread_num];
        aligned_buffer_ = new char*[thread_num];

        iocb** global_iocb_buffers = new iocb*[thread_num];
        iocb*** global_iocb_ptrs = new iocb**[thread_num];
        io_event** global_io_events = new io_event*[thread_num];
        AioNode** global_aio_nodes = new AioNode*[thread_num];
        list<AioNode*>** global_free_nodes = new list<AioNode*>*[thread_num];

        auto start = high_resolution_clock::now();
        for (uint32_t i = 0; i < thread_num; ++i) {
            string temp_value_file = value_file_path + to_string(i);

            write_value_file_dp_[i] = open(temp_value_file.c_str(), open_write_file_flag, FILE_PRIVILEGE);

            if (write_value_file_dp_[i] < 0) {
                log_info("Fail to open key-value files.");
                return;
            }

            fallocate(write_value_file_dp_[i], 0, 0, ((size_t)block_size) * write_block_num);
            aligned_buffer_[i] = (char *) memalign(alignment_size, block_size * queue_depth);

            global_iocb_buffers[i] = new iocb[queue_depth];
            global_iocb_ptrs[i] = new iocb*[queue_depth];
            global_io_events[i] = new io_event[queue_depth];
            global_aio_nodes[i] = new AioNode[queue_depth];
            global_free_nodes[i] = new list<AioNode*>();

            for (uint32_t j = 0; j < queue_depth; ++j) {
                global_aio_nodes[i][j].value_buffer_ptr_ = aligned_buffer_[i] + j * block_size;
                global_aio_nodes[i][j].iocb_ptr_ = global_iocb_buffers[i] + j;
                global_free_nodes[i]->push_back(&global_aio_nodes[i][j]);
            }
        }

        auto end = high_resolution_clock::now();
        log_info("Release %.3lf", duration_cast<milliseconds>(end - start).count() / 1000.0);
        vector<thread> workers(thread_num);

        start = high_resolution_clock::now();

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i] = move(thread([write_file_block_offset, block_size, write_block_num, i, queue_depth,
                                      global_iocb_ptrs, global_io_events, global_free_nodes, this]() {
                int local_value_file_dp = write_value_file_dp_[i];
                iocb** iocb_ptrs = global_iocb_ptrs[i];
                io_event* io_events = global_io_events[i];
                list<AioNode*>* free_nodes = global_free_nodes[i];

                char* local_value = new char[block_size];

                aio_context_t aio_ctx = 0;

                if (io_setup(queue_depth, &aio_ctx) < 0) {
                    log_info("Setup fail\n");
                    return;
                }

                // Start to write.
                uint32_t block_num = write_block_num;
                uint32_t submitted_num = 0;
                uint32_t completed_num = 0;

                while (completed_num < block_num) {
                    long ret;

                    uint32_t free_nodes_num = (uint32_t)free_nodes->size();
                    uint32_t remain_block_num = block_num - submitted_num;

                    uint32_t to_submit = min(free_nodes_num, remain_block_num);

                    if (to_submit > 0) {
                        // Submit.
                        for (uint32_t j = 0; j < to_submit; ++j) {
                            AioNode* aio_node = free_nodes->front();
                            free_nodes->pop_front();

                            memcpy(aio_node->value_buffer_ptr_, local_value, block_size);

                            size_t offset = write_file_block_offset[submitted_num + j] * (size_t)(block_size);
                            fill_aio_node(local_value_file_dp, aio_node, offset, block_size, IOCB_CMD_PWRITE);
                            iocb_ptrs[j] = aio_node->iocb_ptr_;
                        }

                        ret = io_submit(aio_ctx, to_submit, iocb_ptrs);

                        if (ret != to_submit) {
                            log_info("Result %d", ret);
                            return;
                        }

                        submitted_num += to_submit;
                    }

                    // Get completed events.
                    uint32_t in_flight = submitted_num - completed_num;
                    uint32_t expected = (4 <= in_flight ? 4 : in_flight);
                    ret = io_getevents(aio_ctx, expected, in_flight, io_events, NULL);
                    if (ret < 0) {
                        log_info("Get error.");
                        return;
                    }

                    // Handle the completed events.
                    uint32_t to_complete = ret;
                    if (to_complete > 0) {
                        for (uint32_t j = 0; j < to_complete; ++j) {
                            io_event *complete_event = &io_events[j];
                            if (complete_event->res2 != 0 || complete_event->res != block_size) {
                                log_info("Return error.\n");
                                return;
                            }

                            AioNode *aio_node = (AioNode *) complete_event->data;
                            free_nodes->push_back(aio_node);
                        }

                        completed_num += ret;
                    }
                }

                delete[] local_value;
            }));
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i].join();
        }

        end = high_resolution_clock::now();
        log_info("Step one %.3lf", duration_cast<milliseconds>(end - start).count() / 1000.0);

        for (uint32_t i = 0; i < thread_num; ++i) {
            fsync(write_value_file_dp_[i]);
            close(write_value_file_dp_[i]);
            delete[] aligned_buffer_[i];
            delete[] global_aio_nodes[i];
            delete[] global_io_events[i];
            delete[] global_iocb_buffers[i];
            delete[] global_iocb_ptrs[i];
            delete global_free_nodes[i];
        }

        delete[] write_value_file_dp_;
        delete[] aligned_buffer_;
        delete[] global_aio_nodes;
        delete[] global_io_events;
        delete[] global_iocb_buffers;
        delete[] global_iocb_ptrs;
        delete[] global_free_nodes;

        end = high_resolution_clock::now();
        log_info("Step one %.3lf end.", duration_cast<milliseconds>(end - start).count() / 1000.0);

    }

    void EngineRace::Benchmark() {
        const size_t value_file_size = (size_t) VALUE_SIZE * KEY_VALUE_MAX_COUNT_PER_THREAD;
        // const size_t value_file_size = (size_t) VALUE_SIZE * 100;
        vector<uint32_t> block_size_config = {4096 * 4};
        vector<uint32_t> thread_num_config = {64};
        vector<uint32_t> queue_depth_config = {32};
        uint32_t flag_config_num = 1;
        vector<int> write_file_flags_config = {O_CREAT | O_WRONLY | O_DIRECT};

        uint32_t count = 0;
        log_info("Close file..");
        for (uint32_t block_size : block_size_config) {
            uint32_t block_num = (uint32_t)(value_file_size / block_size);
            uint32_t* file_block_offset = new uint32_t[block_num];

            for (uint32_t i = 0; i < block_num; ++i) {
                file_block_offset[i] = i;
            }

            for (uint32_t thread_num : thread_num_config) {
                for (uint32_t flag_config = 0; flag_config < flag_config_num; ++flag_config) {
                    int write_file_flags = write_file_flags_config[flag_config];
                    for (uint32_t queue_depth : queue_depth_config) {
                        log_info("%d", count++);
                        TestDevice(write_file_flags, file_block_offset, block_num, thread_num, block_size,
                                       4096, queue_depth);
                    }
                }
            }

            delete[] file_block_offset;
        }

        log_info("Close file end..");
    }
}  // namespace polar_race
