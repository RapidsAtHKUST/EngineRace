//
// Created by yche on 10/27/18.
//
#include <assert.h>
#include <stdio.h>
#include <string>
#include "include/engine.h"

static const char kEnginePath[] = "/tmp/test_engine";
//static const char kEnginePath[] = "/home/yche/mmap";
//static const char kDumpPath[] = "/tmp/test_dump";
static const char kDumpPath[] = "/tmp/test_dump";

using namespace polar_race;

class DumpVisitor : public Visitor {
public:
    DumpVisitor(int *kcnt)
            : key_cnt_(kcnt) {}

    ~DumpVisitor() {}

    void Visit(const PolarString &key, const PolarString &value) {
        printf("Visit %s --> %s\n", key.data(), value.data());
        (*key_cnt_)++;
    }

private:
    int *key_cnt_;
};

int main() {
    Engine *engine = NULL;

    RetCode ret = Engine::Open(kEnginePath, &engine);
    assert (ret == kSucc);

//#pragma omp parallel num_threads(1)
    {
        ret = engine->Write("aaaaaaaa", "aaaaaaaaaaa");
        assert (ret == kSucc);
        ret = engine->Write("aaaaaaaa", "111111111111111111111111111111111111111111");
        ret = engine->Write("aaaaaaaa", "2222222");
        ret = engine->Write("aaaaaaaaaa", "4");

        ret = engine->Write("bbbbbbbb", "bbbbbbbbbbbb");
        assert (ret == kSucc);

        ret = engine->Write("ccdddddd", "cbbbbbbbbbbbb");
        std::string value;
        ret = engine->Read("aaaaaaaa", &value);
        printf("Read aaa value: %s\n", value.c_str());

        ret = engine->Read("bbbbbbbb", &value);
        assert (ret == kSucc);
        printf("Read bbb value: %s\n", value.c_str());

        int key_cnt = 0;
        DumpVisitor vistor(&key_cnt);
        ret = engine->Range("b", "", vistor);
        assert (ret == kSucc);
        printf("Range key cnt: %d\n", key_cnt);
    };

    delete engine;

    return 0;
}

