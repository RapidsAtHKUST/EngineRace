// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_

#include <string>
#include <mutex>

#include "include/engine.h"
#include "mmap_hash_map.h"

namespace polar_race {
    using namespace std;

    class EngineRace : public Engine {
    public:
        int value_redis_fd_;
        int value_redis_read_only_fd_;
        mmap_hash_map *mmap_hash_map_arr_;
        mutex *mutex_arr_;
    public:
        static RetCode Open(const std::string &name, Engine **eptr);

        explicit EngineRace(const std::string &dir) {

        }

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
