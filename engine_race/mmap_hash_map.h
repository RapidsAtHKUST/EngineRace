#pragma once

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <zconf.h>

#include <cstdint>
#include <string>
#include <cstring>
#include <cassert>

#include <iostream>

using namespace std;

// https://github.com/AlgorithmPlayer/KeyValuePlay/blob/master/other_topers_codes/5_blahgeek.h
asm(
".text\n"
".p2align 4,,15\n"
".globl    MurmurHash2\n"
".type    MurmurHash2, @function\n"
"MurmurHash2:\n"
".cfi_startproc\n"
"cmpl    $3, %esi\n"
"movl    %esi, %eax\n"
"jle    .L2_mmh\n"
"leal    -4(%rsi), %r9d\n"
"movl    %r9d, %r8d\n"
"shrl    $2, %r8d\n"
"movl    %r8d, %eax\n"
"leaq    4(%rdi,%rax,4), %rcx\n"
".p2align 4,,10\n"
".p2align 3\n"
".L3_mmh:\n"
"imull    $1540483477, (%rdi), %eax\n"
"addq    $4, %rdi\n"
"movl    %eax, %edx\n"
"shrl    $24, %edx\n"
"xorl    %edx, %eax\n"
"imull    $1540483477, %esi, %edx\n"
"imull    $1540483477, %eax, %esi\n"
"xorl    %edx, %esi\n"
"cmpq    %rcx, %rdi\n"
"jne    .L3_mmh\n"
"negl    %r8d\n"
"leal    (%r9,%r8,4), %eax\n"
".L2_mmh:\n"
"cmpl    $2, %eax\n"
"je    .L5_mmh\n"
"cmpl    $3, %eax\n"
"je    .L6_mmh\n"
"cmpl    $1, %eax\n"
"je    .L7_mmh\n"
".L4_mmh:\n"
"movl    %esi, %eax\n"
"shrl    $13, %eax\n"
"xorl    %eax, %esi\n"
"imull    $1540483477, %esi, %eax\n"
"movl    %eax, %esi\n"
"shrl    $15, %esi\n"
"xorl    %esi, %eax\n"
"ret\n"
".p2align 4,,10\n"
".p2align 3\n"
".L6_mmh:\n"
"movzbl    2(%rdi), %eax\n"
"sall    $16, %eax\n"
"xorl    %eax, %esi\n"
".L5_mmh:\n"
"movzbl    1(%rdi), %eax\n"
"sall    $8, %eax\n"
"xorl    %eax, %esi\n"
".L7_mmh:\n"
"movzbl    (%rdi), %eax\n"
"xorl    %eax, %esi\n"
"imull    $1540483477, %esi, %esi\n"
"jmp    .L4_mmh\n"
".cfi_endproc\n"
);

extern "C" uint32_t MurmurHash2(const void *, int);

struct node {
    int8_t length_;
    char key_str_[8];
    int32_t value_idx_;
};

inline bool file_exists(const char *file_name) {
    struct stat buffer;
    return (stat(file_name, &buffer) == 0);
}


#define SLOT_NUM ((1 << 20))
#define SLOT_MASK ((1 << 20) - 1)

class mmap_hash_map {
    node *hash_table_;
    int table_fd;

public:
    void open_mmap(const char *file_name) {
        bool is_exists = file_exists(file_name);
//        bool is_exists = false;
        table_fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        auto buffer_size = SLOT_NUM * sizeof(node);
        ftruncate(table_fd, buffer_size);
        hash_table_ = (node *)
                mmap(nullptr, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, table_fd, 0);
        if (!is_exists) {
            memset(hash_table_, 0, buffer_size);
        }
    }

    explicit mmap_hash_map(const char *file_name) {
        open_mmap(file_name);
    }

    ~mmap_hash_map() {
        munmap(hash_table_, SLOT_NUM * sizeof(node));
        close(table_fd);
    }

    node *find(const char *key, int8_t len) {
        auto hash = MurmurHash2(key, len);
        auto index = hash & SLOT_MASK;
        node *obj = &hash_table_[index];
        for (; obj->length_ != 0;) {
            cout << obj->length_ << endl;
            // Case 1st: already exists (len == obj->length_), and is equal to
            if (len == obj->length_ && memcmp(key, obj->key_str_, len) == 0) { return obj; }

            // Case 2nd: linear probing
            index++;
            index &= SLOT_MASK;
            obj = &hash_table_[index];
        }
        return obj;
    }

    void insert(node *obj, const char *key, int8_t len, int32_t assign_idx) {
        obj->length_ = len;
        memcpy(obj->key_str_, key, sizeof(char) * len);
        obj->value_idx_ = assign_idx;
    }

    void put(const char *key, int8_t len, int32_t assign_idx) {
        node *tmp = find(key, len);
        insert(tmp, key, len, assign_idx);
    }
};
