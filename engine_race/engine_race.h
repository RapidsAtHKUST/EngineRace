// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>
#include <atomic>
#include <list>

#include "include/engine.h"
#include "barrier.h"
#include "thread_pool.h"

#define NUM_THREADS (64)
#define TOTAL_COUNT (64000000)
#define VALUE_SIZE (4096)
#define FILESYSTEM_BLOCK_SIZE (4096)
#define FILE_PRIVILEGE (0644)
#define TMP_KEY_BUFFER_SIZE (1024)
#define TMP_VALUE_BUFFER_SIZE (4)
#define TO_UINT64(buffer) (*(uint64_t*)(buffer))
#define WRITE_BARRIER_NUM (16)
#define READ_BARRIER_NUM (32)

#define VAL_BUCKET_DIGITS (10)
#define VAL_BUCKET_NUM (1 << VAL_BUCKET_DIGITS)

#define KEY_BUCKET_DIGITS (VAL_BUCKET_DIGITS)      // must be the same for the range query
#define KEY_BUCKET_NUM (1 << KEY_BUCKET_DIGITS)

#define MAX_BUFFER_NUM (4u)
#define IO_POOL_SIZE (1u)
#define KEY_IO_POOL_SIZE (8u)

#define POSTPONE_READ

namespace polar_race {
    using namespace std;

    struct KeyEntry {
        uint64_t key_;
        uint32_t value_offset_;
    }__attribute__((packed));

    bool operator<(KeyEntry l, KeyEntry r);

    class EngineRace : public Engine {
    public:
        int key_meta_file_dp_;
        uint32_t *mmap_key_meta_cnt_;

        int val_meta_file_dp_;
        uint32_t *mmap_val_meta_cnt_;

        int *write_key_file_dp_;
        int *write_key_buffer_file_dp_;
        KeyEntry **mmap_key_aligned_buffer_;

        mutex *key_mtx_;
        mutex *val_mtx_;

        int *write_value_file_dp_;
        int *write_value_buffer_file_dp_;
        char **mmap_value_aligned_buffer_;

        char **aligned_read_buffer_;

        Barrier barrier_;
        Barrier read_barrier_;
        Barrier *range_barrier_ptr_;

        vector<KeyEntry *> index_;

        volatile bool *is_sorted_;
        volatile bool is_range_init_;
        string dir_;

        // Range Related
        vector<pair<PolarString *, PolarString *>> polar_str_pairs_;

        mutex range_mtx_;
        condition_variable range_init_cond_;
        vector<char *> value_shared_buffers_;

        vector<shared_future<void>> futures_;
        vector<shared_future<void>> key_futures_;
        mutex total_time_mtx_;
        double total_time_;

        double wait_get_time_;
        double enqueue_time_;
        uint64_t val_buffer_max_size_;
        ThreadPool *range_io_worker_pool_;
        ThreadPool *key_io_worker_pool_;

        mutex *bucket_mutex_arr_;
        condition_variable *bucket_cond_var_arr_;
        bool *bucket_is_ready_read_;
        atomic_int *bucket_consumed_num_;
        int32_t total_range_num_threads_;

    public:
        static RetCode Open(const std::string &name, Engine **eptr);

        explicit EngineRace(const std::string &dir);

        ~EngineRace() override;

        RetCode Write(const PolarString &key,
                      const PolarString &value) override;

        RetCode Read(const PolarString &key,
                     std::string *value) override;

        /*
         * NOTICE: Implement 'Range' in quarter-final,
         *         you can skip it in preliminary.
         */
        RetCode Range(const PolarString &lower,
                      const PolarString &upper,
                      Visitor &visitor) override;

    private:
        void ReadBucketToBuffer(uint32_t bucket_id);

        void InitForRange(int64_t tid);

    private:
        void FlushTmpFiles(string dir);

#ifdef POSTPONE_READ

        void LazyLoadIndex(uint32_t key_par_id);

#endif

        void BuildIndex(string dir);
    };

}  // namespace polar_race

#endif  // ENGINE_RACE_ENGINE_RACE_H_
