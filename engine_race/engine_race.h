// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>
#include <atomic>
#include "include/engine.h"
#include "sparsepp/spp.h"
//#include "google/sparse_hash_map"

#define NUM_THREADS (64)
#define VALUE_SIZE (4096)
#define FILESYSTEM_BLOCK_SIZE (4096)
#define FILE_PRIVILEGE (0644)
#define KEY_VALUE_MAX_COUNT_PER_THREAD (1000000)
#define KEY_READ_BLOCK_COUNT (4096)
#define TO_UINT64(buffer) (*(uint64_t*)(buffer))

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

    class EngineRace : public Engine {
    public:
        int* write_key_file_dp_;
        int* write_value_file_dp_;
        int write_meta_file_dp_;
        char** write_mmap_meta_file_;

//        int* read_key_file_dp_;
//        int* read_value_file_dp_;
//        int read_meta_file_dp_;
//        char** read_mmap_meta_file_;

        spp::sparse_hash_map<int64_t, ValueOffset> index_;
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
        void BuildIndex();
    };

}  // namespace polar_race

#endif  // ENGINE_RACE_ENGINE_RACE_H_
