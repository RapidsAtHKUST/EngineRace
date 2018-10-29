// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>
#include <vector>

#include "include/engine.h"
#include "sparsepp/spp.h"

#define PARTITION_NUM (64)
#define META_INDEX_SIZE (4 * PARTITION_NUM)
//#define META_INDEX_SIZE (4096)
#define INDEX_ENTRY_SIZE (12)
#define INDEX_ENTRY_GROUP_SIZE (80*1024)
#define INDEX_CHUNK_MMAP_SIZE (INDEX_ENTRY_SIZE* INDEX_ENTRY_GROUP_SIZE)

#define ID_SKIP (1100000)
#define NUM_THREADS (64)
#define META_VALUE_SIZE (sizeof(int32_t) * 2 * NUM_THREADS)
#define VALUE_SIZE (4096)
#define VALUE_ENTRY_GROUP_SIZE (256)
#define VALUE_CHUNK_MMAP_SIZE (VALUE_SIZE* VALUE_ENTRY_GROUP_SIZE)

#define FILE_PRIVILEGE (S_IRUSR | S_IWUSR)

namespace polar_race {
    using namespace std;
    using namespace spp;

    struct ValueMetaEntry {
        int32_t beg_idx;
        int32_t end_idx;
    };

    class EngineRace : public Engine {
    public:
        int index_meta_fd_;
        int *index_file_fd_arr_;
        int32_t *mmap_hash_meta_count_arr_;                         // index-meta
        pair<int64_t, int32_t> ** mmap_index_entry_arr_;      // index-entry
        vector<sparse_hash_map<int64_t, int32_t>> hash_map_arr_;    // in-memory
        mutex *hash_map_mutex_arr_;                                 // in-memory locks

        int value_meta_fd_;
        int value_write_only_fd_;
        int value_read_only_fd_;
        ValueMetaEntry *mmap_value_meta_id_pair_arr_;       // value-meta
        char ** mmap_value_entry_arr_;                       // value-entry
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
