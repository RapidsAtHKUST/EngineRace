// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>
#include <atomic>

#include "include/engine.h"
#include "barrier.h"

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

#define VAL_SHARED_BUFFER_SIZE (500 * 1024 * 1024)

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

        vector<KeyEntry *> index_;
        char **aligned_read_buffer_;

        Barrier barrier_;
        Barrier read_barrier_;
        Barrier range_barrier_;

        volatile bool *is_sorted_;
        volatile bool is_range_init_;
        string dir_;
        vector<pair<PolarString *, PolarString *>> polar_str_pairs_;

        char* value_shared_buffer_;
        volatile bool * is_loaded_;
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
        void FlushTmpFiles(string dir);

        void LazyLoadIndex(uint32_t key_par_id);

        void BuildIndex(string dir);
    };

}  // namespace polar_race

#endif  // ENGINE_RACE_ENGINE_RACE_H_
