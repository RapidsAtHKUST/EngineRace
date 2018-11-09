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
            tmp_value_buf_size_(NUM_THREADS, 5) {
        clock_end = high_resolution_clock::now();
        log_info("Start init DB, mem usage: %s KB, time: %.3lf s, ts: %.3lf s", FormatWithCommas(getValue()).c_str(),
                 duration_cast<milliseconds>(clock_end - clock_start).count() / 1000.0,
                 std::chrono::duration_cast<std::chrono::milliseconds>(clock_end.time_since_epoch()).count() / 1000.0);
        const string meta_file_path = dir + "/" + meta_file_name;
        const string key_file_path = dir + "/" + key_file_name;
        const string value_file_path = dir + "/" + value_file_name;
        const string tmp_value_file_path = dir + "/" + value_buffer_file_name;
        const string tmp_key_file_path = dir + "/" + key_buffer_file_name;

        const size_t key_file_size = sizeof(KeyEntry) * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;
        const size_t value_file_size = VALUE_SIZE * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;


        for (int i = 0; i < NUM_THREADS; i++) {
            if (i % 2 == 0) {
                tmp_value_buf_size_[i] = 7;
            }
        }
//        const size_t tmp_buffer_value_file_size = VALUE_SIZE * (size_t) TMP_VALUE_BUFFER_SIZE;
        const size_t tmp_buffer_key_file_size = sizeof(KeyEntry) * (size_t) TMP_KEY_BUFFER_SIZE;

        write_key_file_dp_ = new int[NUM_THREADS];
        write_value_file_dp_ = new int[NUM_THREADS];
        write_value_buffer_file_dp_ = new int[NUM_THREADS];
        write_key_buffer_file_dp_ = new int[NUM_THREADS];

        mmap_value_aligned_buffer_ = new char *[NUM_THREADS];
        mmap_key_aligned_buffer_ = new char *[NUM_THREADS];

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
                    mmap_key_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_key_file_size, \
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
                mmap_key_aligned_buffer_[i] = (char *) mmap(nullptr, tmp_buffer_key_file_size, \
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
                munmap(mmap_key_aligned_buffer_[i], sizeof(KeyEntry) * (size_t) TMP_KEY_BUFFER_SIZE);
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
        static thread_local char *value_buffer = mmap_value_aligned_buffer_[tid];
        static thread_local char *key_buffer = mmap_key_aligned_buffer_[tid];
        static thread_local uint32_t TMP_VALUE_BUFFER_SIZE = tmp_value_buf_size_[tid];

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
        KeyEntry key_entry{};
        key_entry.key_ = TO_UINT64(key.data());
        key_entry.value_offset_.partition_ = tid;
        key_entry.value_offset_.block_offset_ = local_block_offset;
        uint32_t key_buffer_offset = (local_block_offset % TMP_KEY_BUFFER_SIZE) * sizeof(KeyEntry);
        memcpy(key_buffer + key_buffer_offset, &key_entry, sizeof(KeyEntry));
        if (((local_block_offset + 1) % TMP_KEY_BUFFER_SIZE) == 0) {
            pwrite(local_key_file, key_buffer, sizeof(KeyEntry) * TMP_KEY_BUFFER_SIZE,
                   ((uint64_t) local_block_offset - (TMP_KEY_BUFFER_SIZE - 1)) * sizeof(KeyEntry));
        }

        // Update the meta data.
        local_block_offset += 1;
        mmap_local_meta_file_[0] = local_block_offset;
        mmap_local_meta_file_[get_partition_id(key_entry.key_) + 1]++;
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
        auto it = lower_bound(index_[partition_id], index_[partition_id] + total_cnt_[partition_id],
                              tmp, [](KeyEntry l, KeyEntry r) {
                    return l.key_ < r.key_;
                });

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
                size_t write_length = (entry_counts[tid] % TMP_KEY_BUFFER_SIZE) * sizeof(KeyEntry);
                size_t write_offset = static_cast<uint64_t>(entry_counts[tid] / TMP_KEY_BUFFER_SIZE *
                                                            TMP_KEY_BUFFER_SIZE) * sizeof(KeyEntry);
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
}  // namespace polar_race
