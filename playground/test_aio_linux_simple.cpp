#define _GNU_SOURCE        /* syscall() is not POSIX */

#include <unistd.h>        /* for syscall() */
#include <fcntl.h>
#include <signal.h>
#include <poll.h>

#include <sys/syscall.h>    /* for __NR_* definitions */
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <linux/aio_abi.h>    /* for AIO types and constants */

#include <malloc.h>
#include <cstdio>        /* for perror() */
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cassert>

#include "log.h"

#define RING_SIZE (16)
#define BUF_SIZE (4096)
#define TESTFILE_SIZE (BUF_SIZE * 4096)

inline void asyio_prep_preadv(struct iocb *iocb, int fd, struct iovec *iov,
                              int nr_segs, int64_t offset, int afd) {
    memset(iocb, 0, sizeof(*iocb));
    iocb->aio_fildes = fd;
    iocb->aio_lio_opcode = IOCB_CMD_PREADV;
    iocb->aio_reqprio = 0;
    iocb->aio_buf = (uint64_t) iov;
    iocb->aio_nbytes = nr_segs;
    iocb->aio_offset = offset;
    iocb->aio_flags = IOCB_FLAG_RESFD;
    iocb->aio_resfd = afd;
}

inline void asyio_prep_pwritev(struct iocb *iocb, int fd, struct iovec *iov,
                               int nr_segs, int64_t offset, int afd) {
    memset(iocb, 0, sizeof(*iocb));
    iocb->aio_fildes = fd;
    iocb->aio_lio_opcode = IOCB_CMD_PWRITEV;
    iocb->aio_reqprio = 0;
    iocb->aio_buf = (uint64_t) iov;
    iocb->aio_nbytes = nr_segs;
    iocb->aio_offset = offset;
    iocb->aio_flags = IOCB_FLAG_RESFD;
    iocb->aio_resfd = afd;
}

inline void asyio_prep_pread(struct iocb *iocb, int fd, void *buf,
                             int nr_segs, int64_t offset, int afd) {
    memset(iocb, 0, sizeof(*iocb));
    iocb->aio_fildes = fd;
    iocb->aio_lio_opcode = IOCB_CMD_PREAD;
    iocb->aio_reqprio = 0;
    iocb->aio_buf = (uint64_t) buf;
    iocb->aio_nbytes = nr_segs;
    iocb->aio_offset = offset;
    iocb->aio_flags = IOCB_FLAG_RESFD;
    iocb->aio_resfd = afd;
}

inline void asyio_prep_pwrite(struct iocb *iocb, int fd, void const *buf,
                              int nr_segs, int64_t offset, int afd) {
    memset(iocb, 0, sizeof(*iocb));
    iocb->aio_fildes = fd;
    iocb->aio_lio_opcode = IOCB_CMD_PWRITE;
    iocb->aio_reqprio = 0;
    iocb->aio_buf = (uint64_t) buf;
    iocb->aio_nbytes = nr_segs;
    iocb->aio_offset = offset;
    iocb->aio_flags = IOCB_FLAG_RESFD;
    iocb->aio_resfd = afd;
}

inline long io_setup(unsigned nr_reqs, aio_context_t *ctx) {
    return syscall(__NR_io_setup, nr_reqs, ctx);
}

inline long io_destroy(aio_context_t ctx) {
    return syscall(__NR_io_destroy, ctx);
}

inline long io_submit(aio_context_t ctx, long n, struct iocb **paiocb) {
    return syscall(__NR_io_submit, ctx, n, paiocb);
}

inline long io_cancel(aio_context_t ctx, struct iocb *aiocb,
                      struct io_event *res) {
    return syscall(__NR_io_cancel, ctx, aiocb, res);
}

inline long io_getevents(aio_context_t ctx, long min_nr, long nr,
                         struct io_event *events, struct timespec *tmo) {
    return syscall(__NR_io_getevents, ctx, min_nr, nr, events, tmo);
}

inline int eventfd(int count) {
    return syscall(__NR_eventfd, count);
}

static long waitasync(int afd, int timeo) {
    struct pollfd pfd;

    pfd.fd = afd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    if (poll(&pfd, 1, timeo) < 0) {
        perror("poll");
        return -1;
    }
    if ((pfd.revents & POLLIN) == 0) {
        fprintf(stderr, "no results completed\n");
        return 0;
    }

    return 1;
}

void TestWrite() {
    // 1st: event fd
    int afd;
    if ((afd = eventfd(0)) == -1) {
        perror("eventfd");
    }

    // 2nd: ctx, io_setup
    aio_context_t ctx;

    // set io control block
    auto ring_iocb = (iocb *) malloc(sizeof(iocb) * RING_SIZE);
    auto ring_piocb = (struct iocb **) malloc(RING_SIZE * sizeof(struct iocb *));

    memset(ring_iocb, 0, sizeof(iocb) * RING_SIZE);

    auto events = (io_event *) malloc(sizeof(io_event) * RING_SIZE);

    auto ring_buffer = (char *) memalign(4096, sizeof(char) * BUF_SIZE * RING_SIZE);
    memset(ring_iocb, 0, sizeof(iocb) * RING_SIZE);

    // io-setup
    auto ret = io_setup(128, &ctx);
    if (ret < 0) {
        log_error("io_setup error");
    }
    // open-file
    int fd = open("test-file", O_CREAT | O_WRONLY | O_DIRECT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        log_error(" Error at open(): %s\n", strerror(errno));
        exit(1);
    }
    ftruncate(fd, TESTFILE_SIZE);
    fcntl(afd, F_SETFL, fcntl(afd, F_GETFL, 0) | O_NONBLOCK);

    int pending_cnt_ = 0;
    int next_id = 0;

    struct timespec tmo{};
    tmo.tv_sec = 0;
    tmo.tv_nsec = 0;
    for (int i = 0; i < TESTFILE_SIZE / BUF_SIZE; i++) {
        if (pending_cnt_ >= RING_SIZE) {
            while (!waitasync(afd, -1));
//            int eval;
//            if (read(afd, &eval, sizeof(eval)) <= 0)
//                log_error("read");
            auto r = io_getevents(ctx, 1, 1, events, &tmo);
            pending_cnt_ -= r;
        }

        char *buffer = ring_buffer + BUF_SIZE * next_id;
        for (auto j = 0; j < BUF_SIZE; j++) {
            buffer[j] = static_cast<char>(j * j + i * i);
        }
        asyio_prep_pwrite(ring_iocb + next_id, fd, (const void *) buffer, BUF_SIZE, static_cast<size_t>(BUF_SIZE) * i,
                          afd);

        ring_piocb[next_id] = ring_iocb + next_id;
        if (io_submit(ctx, 1L, ring_piocb + next_id) == -1) {
            log_error("Error at aio_write(): %s\n", strerror(errno));
        }
        next_id = (next_id + 1) % RING_SIZE;
        pending_cnt_++;
//        log_info("ok\n");

        while (pending_cnt_ > 0) {
            while (!waitasync(afd, -1));
//            int eval;
//            if (read(afd, &eval, sizeof(eval)) <= 0)
//                log_error("read");
            auto r = io_getevents(ctx, 1, 1, events, &tmo);
            pending_cnt_ -= r;
        }
    }
    // io-destroy, close fds
    io_destroy(ctx);
    close(fd);
    close(afd);
}

void TestRead() {
    /* check the values written */
    int fd = open("test-file", O_RDONLY, S_IRUSR | S_IWUSR);
    char buf[BUF_SIZE];

    for (int i = 0; i < TESTFILE_SIZE / BUF_SIZE; i++) {
        ssize_t ret = pread(fd, buf, BUF_SIZE, i * BUF_SIZE);
        if (ret != BUF_SIZE) {
            printf(" Error at pread() : %s\n", strerror(errno));
            close(fd);
            exit(2);
        }
        for (auto j = 0; j < BUF_SIZE; j++) {
//            printf("%d, %d\n", i, j);
            assert(buf[j] == static_cast<char>(j * j + i * i));
        }
    }
    close(fd);
    printf("Test PASSED\n");
}

int main(int argc, char *argv[]) {
    TestWrite();
    TestRead();
    return 0;
}