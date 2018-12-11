## Disclaimer

此项目仅供学习与交流使用，请遵循MIT协议，如果您在任何项目中使用相关代码，请保留此项目的LICENSE文件。

## Code Review文档

[Code-Reivew文档](documents/ReadMe.md)

## 目录结构

项目代码在两个目录中 [engine_race](engine_race) 和 [playground](playground)。
浏览代码可以用[clion](https://www.jetbrains.com/clion/)打开项目看。

### KV-DB引擎实现的代码

文件 | 说明
--- | ---
[engine_race/barrier.h](engine_race/barrier.h) | 可以重复使用的barrier，通过generation来支持
[engine_race/blocking_queue.h](engine_race/blocking_queue.h) | 简单的blocking-queue实现 (没有任何性能考虑的)
[engine_race/concurrentqueue.h](engine_race/concurrentqueue.h), [engine_race/blockingconcurrentqueue.h](engine_race/blockingconcurrentqueue.h) | 工业级别的blocking-queue实现 [cameron314/concurrentqueue](https://github.com/cameron314/concurrentqueue)，具体使用参考两个文件中的license
[engine_race/file_util.h](engine_race/file_util.h) | Linux-AIO和文件相关的封装
[engine_race/util.h](engine_race/util.h) | 统计性能调优的信息: `dstat`, `iostat`, 内存占用; 打印timestamp和对应代码行位置util
[engine_race/log.h](engine_race/log.h), [engine_race/log.cc](engine_race/log.cc) | log工具, 稍做修改

* 曾经使用的文件

文件 | 说明
--- | ---
[engine_race/thread_pool.h](engine_race/thread_pool.h) | 简单的线程池实现 [progschj/ThreadPool](https://github.com/progschj/ThreadPool), 在之前版本中使用, 后来改用`std::thread`, `std::promise`, `std::future`代替了
[engine_race/sparsepp](engine_race/sparsepp) | 高效的sparese hashmap实现，特点: 空间占用少, [greg7mdp/sparsepp](https://github.com/greg7mdp/sparsepp)

### 测试的代码

文件 | 说明
--- | ---
[playground/test_engine.cpp](playground/test_engine.cpp) | 测试代码, 三阶段分别使用不同的omp线程池


### Cmake Config文件

文件 | 说明
--- | ---
[CMakeLists.txt](CMakeLists.txt) | 根cmakelist
[playground/CMakeLists.txt](playground/CMakeLists.txt) | playground cmakelist

## 本地测试使用

```zsh
mkdir build && cd build 
cmake ..
make -j
./playground/test_engine_nonoff        
```