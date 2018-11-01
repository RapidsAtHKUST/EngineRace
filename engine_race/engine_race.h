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
#define FILESYSTEM_BLOCK_SIZE (512)
#define FILE_PRIVILEGE (0644)
#define KEY_VALUE_MAX_COUNT_PER_THREAD (1000000)
#define KEY_READ_BLOCK_COUNT (65536)

namespace polar_race {
    using namespace std;
    struct KeyEntry {
        int64_t key_;
        int64_t value_offset_;
    };

    class EngineRace : public Engine {
    public:
        int meta_file_handler_;
        int key_file_handler_;
        int value_file_handler_;
        char* mmap_meta_file_;
        char* mmap_key_file_;
        char* mmap_value_file_;
        char** write_value_buffers_;
        char** read_value_buffers_;

        atomic_int atomic_value_file_block_offset;
        int32_t global_key_block_count_;
        mutex mtx_update_key_file_;

        spp::sparse_hash_map<int64_t, int64_t> index_;

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
