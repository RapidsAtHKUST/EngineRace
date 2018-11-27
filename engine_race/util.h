#pragma once

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <thread>

#include "log.h"

using namespace std;

inline void setThreadSelfAffinity(int core_id) {
//    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
//    assert(core_id >= 0 && core_id < num_cores);
//    if (core_id == 0) {
//        printf("affinity relevant logical cores: %ld\n", num_cores);
//    }
//    core_id = core_id % num_cores;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();
    int ret = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    if (ret != 0) {
        log_error("Set affinity error: %d, %s", ret, strerror(errno));
    }
}

inline std::string exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

inline std::string dstat() {
    std::array<char, 512> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen("dstat -tcdrlmgy --fs", "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    int times = 0;
    int global_times = 0;
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 512, pipe.get()) != nullptr) {
            if (global_times < 100) {
                result += buffer.data();
                times++;
                global_times++;
                if (times >= 5) {
                    log_debug(
                            "\n----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg---"
                            " ------memory-usage----- ---paging-- ---system-- --filesystem-"
                            "\n     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | "
                            "used  buff  cach  free|  in   out | int   csw |files  inodes"
                            "\n%s", result.c_str());
                    times = 0;
                    result.clear();
                }
            } else {
                log_debug(
                        "\n----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg---"
                        " ------memory-usage----- ---paging-- ---system-- --filesystem-"
                        "\n     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | "
                        "used  buff  cach  free|  in   out | int   csw |files  inodes"
                        "\n%s", buffer.data());
            }
        }
    }
    return result;
}

inline void DstatCorountine() {
    thread my_coroutine = thread([]() {
        dstat();
    });
    my_coroutine.detach();
}

inline int parseLine(char *line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char *p = line;
    while (*p < '0' || *p > '9') p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
}

inline int getValue() { //Note: this value is in KB!
    FILE *file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

inline void PrintMemFree() {
    log_info("Consumption: %d KB, Free: \n%s", getValue(), exec("free").c_str());
}