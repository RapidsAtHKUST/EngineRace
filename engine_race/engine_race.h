// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>

#include "include/engine.h"
#include "sparsepp/spp.h"

#define VALUE_SIZE (4096)
#define NUM_THREADS (64)
#define KEY_SIZE (8)
#define PARTITION_NUM (128)
#define META_INDEX_SIZE (4 * PARTITION_NUM)
#define META_VALUE_SIZE (4 * NUM_THREADS)
#define ID_SKIP (1100000)
#define INDEX_ENTRY_GROUP (80*1024)

namespace polar_race {
    using namespace std;
    using namespace spp;

    class EngineRace : public Engine {
    public:
        int value_write_only_fd_;
        int value_read_only_fd_;

        int index_meta_fd_;
        int* index_file_fd_arr_;
        int value_meta_fd_;

        int32_t *mmap_hash_count_arr_;
        pair<int32_t, int32_t> *mmap_value_id_pair_arr_;    // [beg, end)
        pair<int64_t, int32_t> *mmap_index_entry_arr_;      // key, vid
        vector<sparse_hash_map<int64_t, int32_t>> hash_map_arr_;
        mutex *hash_map_mutex_arr_;
    public:
        static RetCode Open(const std::string &name, Engine **eptr);

        explicit EngineRace(const std::string &dir);

        ~EngineRace();

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

    };

}  // namespace polar_race

#endif  // ENGINE_RACE_ENGINE_RACE_H_
