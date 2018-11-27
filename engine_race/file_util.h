#pragma once

#include <cstring>
#include <cstdlib>
#include <cstdint>

#include <fcntl.h>
#include <unistd.h>
#include <linux/aio_abi.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/stat.h>

static inline long io_setup(unsigned maxevents, aio_context_t *ctx) {
    return syscall(SYS_io_setup, maxevents, ctx);
}

static inline long io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) {
    return syscall(SYS_io_submit, ctx, nr, iocbpp);
}

static inline long io_getevents(aio_context_t ctx, long min_nr, long nr,
                                struct io_event *events, struct timespec *timeout) {
    return syscall(SYS_io_getevents, ctx, min_nr, nr, events, timeout);
}

static inline long io_destroy(aio_context_t ctx) {
    return syscall(SYS_io_destroy, ctx);
}

static inline void fill_aio_node(int fd, iocb *iocb_ptr, char *buffer,
                                 size_t offset, size_t buffer_size, uint16_t operation) {
    memset(iocb_ptr, 0, sizeof(iocb));
    iocb_ptr->aio_buf = (uintptr_t) buffer;
    iocb_ptr->aio_data = (uintptr_t) iocb_ptr;
    iocb_ptr->aio_fildes = fd;
    iocb_ptr->aio_lio_opcode = operation;
    iocb_ptr->aio_reqprio = 0;
    iocb_ptr->aio_nbytes = buffer_size;
    iocb_ptr->aio_offset = offset;
}


inline bool file_exists(const char *file_name) {
    struct stat buffer;
    return (stat(file_name, &buffer) == 0);
}

inline size_t file_size(const char *file_name) {
    struct stat st;
    stat(file_name, &st);
    size_t size = st.st_size;
    return size;
}
