// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>
#include <atomic>
#include "include/engine.h"
//#include "sparsepp/spp.h"
//#include "google/sparse_hash_map"

//#include "sorted_array.h"

#include "barrier.h"

#define NUM_THREADS (64)
#define VALUE_SIZE (4096)
#define FILESYSTEM_BLOCK_SIZE (4096)
#define FILE_PRIVILEGE (0644)
#define KEY_VALUE_MAX_COUNT_PER_THREAD (1000000)
//#define TMP_VALUE_BUFFER_SIZE (4)
#define TMP_KEY_BUFFER_SIZE (512)
#define KEY_READ_BLOCK_COUNT (8192)
#define TO_UINT64(buffer) (*(uint64_t*)(buffer))
#define WRITE_BARRIER_NUM (16)
#define READ_BARRIER_NUM (32)

namespace polar_race {
    using namespace std;
    struct ValueOffset {
        uint32_t partition_;
        uint32_t block_offset_;
    };

    struct KeyEntry {
        uint64_t key_;
        ValueOffset value_offset_;

    };

    bool operator<(KeyEntry l, KeyEntry r);

    class EngineRace : public Engine {
    public:
        int *write_key_file_dp_;
        int *write_value_file_dp_;
        int *write_value_buffer_file_dp_;
        int *write_key_buffer_file_dp_;
        int write_meta_file_dp_;

        uint32_t **write_mmap_meta_file_;
        char **mmap_value_aligned_buffer_;
        uint64_t **mmap_key_aligned_buffer_;

        char **aligned_buffer_;

        vector<KeyEntry *> index_;
        vector<uint32_t> total_cnt_;
        vector<uint32_t> tmp_value_buf_size_;
        vector<int64_t> lower_bound_cost_;
        Barrier barrier_;
        Barrier read_barrier_;
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
        void BuildIndex(string dir);
    };

}  // namespace polar_race

#endif  // ENGINE_RACE_ENGINE_RACE_H_
