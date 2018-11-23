// Author: Lipeng WANG (wang.lp@outlook.com)
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <malloc.h>
#include <array>
#include <cassert>
#include <cstdlib>
#include <random>
#include <thread>
#include <algorithm>
#include <sys/time.h>
#include "../engine_race/log.h"

#define THREAD_NUM (64u)
#define BUFF_SIZE (4096 * 1024 * 128)
#define BUCKET_NUM (100)
#define BUCKET_BUFF_NUM (10)

char* global_buff[BUCKET_BUFF_NUM];

std::vector<size_t > read_offsets[BUCKET_NUM];

uint64_t getCurrentUs(){
    struct timeval time;
    if (gettimeofday(&time, NULL)) {
        //  Handle error
        return 0;
    }
    return time.tv_sec * 1000000 + time.tv_usec;
}


void thread_work(int tid){
    uint64_t dummy_sum = 0;
    char internal_buff[4096];
    for(size_t bucket_idx = 0 ; bucket_idx < BUCKET_NUM; bucket_idx++){
        for(size_t idx = 0; idx < read_offsets[bucket_idx].size(); idx++){
            memcpy(internal_buff, global_buff[bucket_idx % BUCKET_BUFF_NUM] + read_offsets[bucket_idx][idx], 4096);
            for(size_t i = 0; i < 4096; i+=16) {
                dummy_sum += internal_buff[i];
            }
        }
    }
    log_info("thread %d exit, sum: %ld", tid, dummy_sum);
}

int main(int argc, char** argv){

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis(0, 127);

    log_info("generate data");
    for(size_t bucket_idx = 0; bucket_idx < BUCKET_BUFF_NUM; bucket_idx++) {
        global_buff[bucket_idx] = (char*)memalign(512, BUFF_SIZE);
        for (size_t i = 0; i < BUFF_SIZE; i += 8) {
            global_buff[bucket_idx][i] = (char) dis(g);
        }
    }

    for (size_t bucket_idx = 0; bucket_idx < BUCKET_NUM; bucket_idx++) {
        for (size_t i = 0; i < BUFF_SIZE / 4096; i++) {
            read_offsets[bucket_idx].push_back(i);
        }
        //no shuffle here
    }

    log_info("start to read (sorted)");

    std::thread tids[THREAD_NUM];
    uint64_t start_time = getCurrentUs();
    for(size_t i = 0; i < THREAD_NUM; i++){
        tids[i] = std::thread(thread_work, i);
    }

    for(size_t i = 0; i < THREAD_NUM; i++){
        tids[i].join();
    }
    uint64_t end_time = getCurrentUs();

    log_info("run time (sorted): %.3lfs\n", (double)(end_time - start_time)/1000000);


    for (size_t bucket_idx = 0; bucket_idx < BUCKET_NUM; bucket_idx++) {
        std::shuffle(read_offsets[bucket_idx].begin(), read_offsets[bucket_idx].end(), g);
    }

    log_info("start to read (shuffled)");
    start_time = getCurrentUs();
    for(size_t i = 0; i < THREAD_NUM; i++){
        tids[i] = std::thread(thread_work, i);
    }
    for(size_t i = 0; i < THREAD_NUM; i++){
        tids[i].join();
    }
    end_time = getCurrentUs();
    log_info("run time (shuffled): %.3lfs", (double)(end_time - start_time)/1000000);

    return 0;
}