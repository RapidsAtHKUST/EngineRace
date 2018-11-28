// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>
#include <atomic>
#include <list>

#include <linux/aio_abi.h>
#include <sys/syscall.h>

#include "sparsepp/spp.h"
#include "include/engine.h"
#include "barrier.h"
#include "thread_pool.h"

#define NUM_THREADS (64)
#define VALUE_SIZE (4096)
#define FILESYSTEM_BLOCK_SIZE (4096)
#define FILE_PRIVILEGE (0644)
#define TMP_KEY_BUFFER_SIZE (512)
#define TMP_VALUE_BUFFER_SIZE (4)
#define TO_UINT64(buffer) (*(uint64_t*)(buffer))

#define WRITE_BARRIER_NUM (16)
#define READ_BARRIER_NUM (32)

#define BUCKET_DIGITS (10)      // must be the same for the range query
#define BUCKET_NUM (1 << BUCKET_DIGITS)

#define NUM_READ_KEY_THREADS (NUM_THREADS)
#define NUM_FLUSH_TMP_THREADS (8u)

#define IO_POOL_SIZE (1u)       // have to be one for aio
#define MAX_RECYCLE_BUFFER_NUM (2u)
#define KEEP_REUSE_BUFFER_NUM (3u)
#define MAX_TOTAL_BUFFER_NUM (MAX_RECYCLE_BUFFER_NUM + KEEP_REUSE_BUFFER_NUM)

#define KEY_READ_BLOCK_COUNT (8192u)
#define FALLOCATE_SIZE (4 * 1024 * 1024)

#define FALLOCATE_POOL_SIZE (32u)
#define MAX_FALLOCATE_RESERVE_SLICE_NUM (1)

#define FALLOCATE_KEY_FILE_SIZE (1024 * 1024)

namespace polar_race {
    using namespace std;

    struct KeyEntry {
        uint64_t key_;
        uint32_t value_offset_;
    }__attribute__((packed));

    bool operator<(KeyEntry l, KeyEntry r);

    class EngineRace : public Engine {
    public:
        int meta_cnt_file_dp_;
        uint32_t *mmap_meta_cnt_;

        int *key_file_dp_;
        int *key_buffer_file_dp_;
        uint64_t **mmap_key_aligned_buffer_;

        int *value_file_dp_;
        int *value_buffer_file_dp_;
        char **mmap_value_aligned_buffer_;

        // Write.
        ThreadPool *fallocate_pool_;
        vector<queue<shared_future<void>>> fallocate_futures_per_bucket_;
        vector<uint32_t> fallocate_slice_id_end_;
        mutex *partition_mtx_;
        Barrier write_barrier_;

        // Read.
        char **aligned_read_buffer_;
        Barrier read_barrier_;

        vector<KeyEntry *> index_;

        // Range.
        volatile bool is_range_init_;
        Barrier *range_barrier_ptr_;
        vector<PolarString *> polar_keys_;

        mutex range_mtx_;
        condition_variable range_init_cond_;
        vector<char *> value_shared_buffers_;

        vector<shared_future<void>> futures_;
        double total_time_;
        double total_io_sleep_time_;

        double wait_get_time_;
        uint64_t val_buffer_max_size_;
        ThreadPool *range_io_worker_pool_;

        // Range Sequential IO.
        mutex *bucket_mutex_arr_;
        condition_variable *bucket_cond_var_arr_;
        bool *bucket_is_ready_read_;
        atomic_int *bucket_consumed_num_;
        int32_t total_range_num_threads_;

        // AIO.
        iocb **iocb_ptrs;
        iocb *iocbs;
        io_event *io_events;
        aio_context_t aio_ctx;
        uint32_t queue_depth;

        list<iocb *> free_nodes;

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
        void FAllocateForBucket(uint32_t par_bucket_id);

    private:
        void ReadBucketToBuffer(uint32_t bucket_id);

        void InitRangeReader();

        void InitAIOContext();

        void InitForRange(int64_t tid);

    private:
        void FlushTmpFiles(string dir);

        void BuildIndex(string dir);
    };

}  // namespace polar_race

#endif  // ENGINE_RACE_ENGINE_RACE_H_
