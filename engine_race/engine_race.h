// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>
#include <atomic>
#include <list>
#include <map>

#include <linux/aio_abi.h>
#include <sys/syscall.h>

#include "include/engine.h"
#include "barrier.h"
#include "thread_pool.h"
#include "blocking_queue.h"

#define TO_UINT64(buffer) (*(uint64_t*)(buffer))

#define FILE_PRIVILEGE (0644)
#define FILESYSTEM_BLOCK_SIZE (4096)
#define NUM_THREADS (64)

// Buffers.
#define TMP_KEY_BUFFER_SIZE (512)
#define TMP_VALUE_BUFFER_SIZE (32)
// Key/Value Files.
#define VALUE_SIZE (4096)

// Buckets.
#define BUCKET_DIGITS (10)   // k-v-buckets must be the same for the range query, (assuming >=10 balanced for uint16_t)
#define BUCKET_NUM (1 << BUCKET_DIGITS)

// Max Bucket Size * BUCKET_NUM.
#define MAX_TOTAL_SIZE (68 * 1024 * 1024)

#define KEY_FILE_DIGITS (6)     // must make sure same bucket in the same file
#define KEY_FILE_NUM (1 << KEY_FILE_DIGITS)
#define MAX_KEY_BUCKET_SIZE (MAX_TOTAL_SIZE / BUCKET_NUM / FILESYSTEM_BLOCK_SIZE * FILESYSTEM_BLOCK_SIZE)

#define VAL_FILE_DIGITS (6)
#define VAL_FILE_NUM (1 << VAL_FILE_DIGITS)  // must make sure same bucket in the same file
#define MAX_VAL_BUCKET_SIZE (MAX_TOTAL_SIZE / BUCKET_NUM / FILESYSTEM_BLOCK_SIZE * FILESYSTEM_BLOCK_SIZE)

// Write.
#define WRITE_BARRIER_NUM (16)
// Read.
#define NUM_READ_KEY_THREADS (NUM_THREADS)
#define NUM_FLUSH_TMP_THREADS (32u)
#define READ_BARRIER_NUM (32)
#define KEY_READ_BLOCK_COUNT (8192u)
// Range.
#define IO_POOL_SIZE (1u)       // have to be one for aio
#define MAX_RECYCLE_BUFFER_NUM (2u)
#define KEEP_REUSE_BUFFER_NUM (3u)
#define MAX_TOTAL_BUFFER_NUM (MAX_RECYCLE_BUFFER_NUM + KEEP_REUSE_BUFFER_NUM)

namespace polar_race {
    using namespace std;

    struct KeyEntry {
        uint64_t key_;
        uint16_t value_offset_;
    }__attribute__((packed));

    struct AIOFileInfo {
        uint32_t file_id_;
        uint64_t file_off_;
    };

    bool operator<(KeyEntry l, KeyEntry r);

    class EngineRace : public Engine {
    public:
        // 1) wal cnt, 2) mmap buffer cnt
        int meta_cnt_file_dp_;
        uint32_t *mmap_meta_cnt_;
        uint32_t *mmap_meta_val_not_flushed_cnt_;

        int *key_file_dp_;
        int key_buffer_file_dp_;
        uint64_t *mmap_key_aligned_buffer_;
        uint64_t **mmap_key_aligned_buffer_view_;

        int *value_file_dp_;
        int value_buffer_file_dp_;
        char *mmap_value_aligned_buffer_;

        // AIO Files.
        int aio_meta_file_dp_;
        AIOFileInfo *mmap_aio_off_;        // Max To Represent No Need to Flush
        int aio_value_buffer_file_dp_;
        char *mmap_aio_value_aligned_buffer_;

        char **mmap_value_aligned_buffer_view_;
        blocking_queue<char *> free_buffers_;
        map<iocb *, const char *> buffer_dict_;

        // AIO Context.
        vector<iocb *> iocbs_tls_;
        vector<io_event *> io_events_tls_;
        vector<aio_context_t> aio_ctx_tls_;

        vector<std::list<iocb * >> free_nodes_tls_;

        // Write.
        mutex *bucket_mtx_;
        Barrier write_barrier_;

        vector<ThreadPool *> writer_value_io_ptr_pool_;

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

        // AIO For Range.
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
        void InitWriteAIOContext();

        void WriteBufferToFile(uint32_t bucket_id, uint32_t fid, uint64_t foff, const char *value_buffer);

        void SyncAIOContext(uint32_t tid);

    private:
        void InitRangeReader();

        void InitRangeAIOContext();

        void InitForRange(int64_t tid);

        void ReadBucketToBuffer(uint32_t bucket_id);

    private:
        void ParallelFlushTmp(int *key_fds, int *val_fds);

        void FlushTmpFiles(string dir);

        void BuildIndex();
    };

}  // namespace polar_race

#endif  // ENGINE_RACE_ENGINE_RACE_H_
