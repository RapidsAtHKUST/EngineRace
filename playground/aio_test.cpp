//
// Created by ssunah on 11/12/18.
//


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <linux/aio_abi.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <list>
#include <algorithm>

using namespace std;

const char* file_name = "test_file.output";
const uint32_t file_size = 1024 * 1024;
const uint32_t page_size = 4096;
const uint32_t queue_depth = 32;

struct AioNode {
    char* value_buffer_ptr_;
    iocb* iocb_ptr_;
};

static inline long
io_setup(unsigned maxevents, aio_context_t *ctx) {
    return syscall(SYS_io_setup, maxevents, ctx);
}

static inline long
io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) {
    return syscall(SYS_io_submit, ctx, nr, iocbpp);
}

static inline long
io_getevents(aio_context_t ctx, long min_nr, long nr,
             struct io_event *events, struct timespec *timeout) {
    return syscall(SYS_io_getevents, ctx, min_nr, nr, events, timeout);
}

static inline long
io_destroy(aio_context_t ctx) {
    return syscall(SYS_io_destroy, ctx);
}

static inline void
fill_aio_node(int fd, AioNode* aio_node, size_t offset, size_t buffer_size, uint16_t operation) {
    memset(aio_node->iocb_ptr_, 0, sizeof(iocb));
    aio_node->iocb_ptr_->aio_buf = (uintptr_t) aio_node->value_buffer_ptr_;
    aio_node->iocb_ptr_->aio_data = (uintptr_t) aio_node;
    aio_node->iocb_ptr_->aio_fildes = fd;
    aio_node->iocb_ptr_->aio_lio_opcode = operation;
    aio_node->iocb_ptr_->aio_reqprio = 0;
    aio_node->iocb_ptr_->aio_nbytes = buffer_size;
    aio_node->iocb_ptr_->aio_offset = offset;
}

void AioWrite() {
    // Open the file.
    int fd = open(file_name, O_CREAT | O_WRONLY | O_DIRECT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        printf("Cannot open file %s\n", file_name);
        exit(-1);
    }
    fallocate(fd, 0, 0, file_size);

    // Create the aio context.
    aio_context_t aio_ctx = 0;

    if (io_setup(queue_depth, &aio_ctx) < 0) {
        printf("Setup fail\n");
        exit(-1);
    }

    // Initialize the memory buffer.
    char* value_buffer_array = (char*)memalign(page_size, page_size * queue_depth);
    iocb* iocb_buffer = new iocb[queue_depth];
    iocb** iocb_ptrs = new iocb*[queue_depth];
    io_event* io_events = new io_event[queue_depth];
    AioNode* aio_nodes = new AioNode[queue_depth];
    // Initialize the aio list.
    list<AioNode*> free_nodes;
    for (uint32_t i = 0; i < queue_depth; ++i) {
        aio_nodes[i].value_buffer_ptr_ = value_buffer_array + i * page_size;
        aio_nodes[i].iocb_ptr_ = iocb_buffer + i;
        free_nodes.push_back(&aio_nodes[i]);
    }

    // Start to write.
    uint32_t block_num = file_size / page_size;
    uint32_t submitted_num = 0;
    uint32_t completed_num = 0;

    while (completed_num < block_num) {
        long ret;

        uint32_t free_nodes_num = (uint32_t)free_nodes.size();
        uint32_t remain_block_num = block_num - submitted_num;

        uint32_t to_submit = min(free_nodes_num, remain_block_num);

        if (to_submit > 0) {
            // Submit.
            for (uint32_t i = 0; i < to_submit; ++i) {
                AioNode* aio_node = free_nodes.front();
                free_nodes.pop_front();

                // Set a value for test.
                uint32_t set_value = submitted_num + i;
                memcpy(aio_node->value_buffer_ptr_, &set_value, sizeof(uint32_t));

                fill_aio_node(fd, aio_node, ((size_t)(submitted_num + i)) * page_size, page_size, IOCB_CMD_PWRITE);
                iocb_ptrs[i] = aio_node->iocb_ptr_;
            }

            ret = io_submit(aio_ctx, to_submit, iocb_ptrs);

            if (ret != to_submit) {
                printf("Add error.\n");
                exit(-1);
            }

            submitted_num += to_submit;
        }

        // Get completed events.
        uint32_t in_flight = submitted_num - completed_num;
        ret = io_getevents(aio_ctx, 0, in_flight, io_events, NULL);
        if (ret < 0) {
            printf("Get error.\n");
            exit(-1);
        }

        // Handle the completed events.
        uint32_t to_complete = ret;
        if (to_complete > 0) {
            for (uint32_t i = 0; i < to_complete; ++i) {
                io_event *complete_event = &io_events[i];
                if (complete_event->res2 != 0 || complete_event->res != page_size) {
                    printf("Return error.\n");
                    exit(-1);
                }

                AioNode *aio_node = (AioNode *) complete_event->data;
                free_nodes.push_back(aio_node);
            }

            completed_num += ret;
            printf("Complete %d\n", completed_num);
        }
    }

    close(fd);
    free(value_buffer_array);
    delete[] iocb_buffer;
    delete[] io_events;
    delete[] iocb_ptrs;
    delete[] aio_nodes;
}

void CheckResult() {
    int fd = open(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        printf("Cannot open file %s\n", file_name);
        exit(-1);
    }

    uint32_t block_num = file_size  / page_size;
    char buffer[page_size];
    for (uint32_t i = 0; i < block_num; ++i) {
        pread(fd, buffer, page_size, i * page_size);
        uint32_t test_value;
        memcpy((void*)&test_value, buffer, sizeof(uint32_t));
        if (test_value != i) {
            printf("Error %d %d", test_value, i);
            exit(-1);
        }
    }
    close(fd);
    printf("Correct");
}
int main() {
    //AioWrite();
    //CheckResult();
    return 0;
}