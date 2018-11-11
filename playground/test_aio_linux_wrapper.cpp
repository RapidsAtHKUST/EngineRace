/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *      The aio_write() function shall write aio_nbytes to the files associated
 *      with aio_fildes from the buffer pointer to by aio_buf.
 *
 * method:
 *
 *      - open file
 *      - write 512 bytes using aio_write
 *      - read 513 bytes
 *      - check read data
 *
 */

//#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <aio.h>
#include <cassert>
#include <linux/aio_abi.h>

#define TNAME "aio_write/1-1.c"
#define BUF_SIZE 512
#define RING_SIZE 16


void TestWrite() {
    char tmpfname[256];
    int fd;

    snprintf(tmpfname, sizeof(tmpfname), "pts_aio_write_1_1_%d", 0);
//    unlink(tmpfname);
    fd = open(tmpfname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf(TNAME " Error at open(): %s\n", strerror(errno));
        exit(1);
    }
//    unlink(tmpfname);

    int pending_cnt_ = 0;
    int next_id_ = 0;
    auto ring_aiocb = (struct aiocb *) malloc(sizeof(aiocb) * RING_SIZE);
    memset(ring_aiocb, 0, sizeof(aiocb) * RING_SIZE);
    auto ring_buffer = (char *) malloc(sizeof(char) * BUF_SIZE * RING_SIZE);
    memset(ring_aiocb, 0, sizeof(struct aiocb) * RING_SIZE);

    for (int i = 0; i < 4096 * 16; i++) {
        if (pending_cnt_ >= RING_SIZE) {
            while (aio_error(&ring_aiocb[next_id_]) == EINPROGRESS);
            pending_cnt_--;
            int err = aio_error(&ring_aiocb[next_id_]);
            ssize_t ret = aio_return(&ring_aiocb[next_id_]);
            if (err != 0) {
                printf(TNAME " Error at aio_error() : %s\n", strerror(err));
                close(fd);
                exit(2);
            }
            if (ret != BUF_SIZE) {
                printf(TNAME " Error at aio_return()\n");
                close(fd);
                exit(2);
            }
        }

        ring_aiocb[next_id_].aio_fildes = fd;
        ring_aiocb[next_id_].aio_offset = BUF_SIZE * i;
        ring_aiocb[next_id_].aio_nbytes = BUF_SIZE;
        ring_aiocb[next_id_].aio_lio_opcode = IOCB_CMD_PWRITE;
        char *buffer = ring_buffer + BUF_SIZE * next_id_;
        ring_aiocb[next_id_].aio_buf = buffer;

        for (auto j = 0; j < BUF_SIZE; j++) {
            buffer[j] = static_cast<char>(j * j + i * i);
        }
        if (aio_write(&ring_aiocb[next_id_]) == -1) {
            printf(TNAME " Error at aio_write(): %s\n", strerror(errno));
            close(fd);
            exit(2);
        }
        next_id_ = (next_id_ + 1) % RING_SIZE;
        pending_cnt_++;
    }
    printf("ok\n");
    while (pending_cnt_ > 0) {
        while (aio_error(&ring_aiocb[next_id_]) == EINPROGRESS);
        pending_cnt_--;
        int err = aio_error(&ring_aiocb[next_id_]);
        ssize_t ret = aio_return(&ring_aiocb[next_id_]);
        if (err != 0) {
            printf(TNAME " Error at aio_error() : %s\n", strerror(err));
            close(fd);
            exit(2);
        }
        if (ret != BUF_SIZE) {
            printf(TNAME " Error at aio_return()\n");
            close(fd);
            exit(2);
        }
    }

}

void TestRead() {
    /* check the values written */
    char tmpfname[256];
    snprintf(tmpfname, sizeof(tmpfname), "pts_aio_write_1_1_%d", 0);

    int fd = open(tmpfname, O_RDONLY, S_IRUSR | S_IWUSR);
    char buf[BUF_SIZE];

    for (int i = 0; i < 4096 * 16; i++) {
        ssize_t ret = pread(fd, buf, BUF_SIZE, i * BUF_SIZE);
        if (ret != BUF_SIZE) {
            printf(TNAME " Error at pread() : %s\n", strerror(errno));
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

int main() {
    TestWrite();
    TestRead();
    return 0;
}