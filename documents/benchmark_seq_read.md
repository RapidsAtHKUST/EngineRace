First test date: 11/05/2018

## 思路



## 实现

### Benchmark Function

* parameters to tune
    * block size or buffer size: `block_size_config`
    * file flags: `write_file_flags_config`,  `read_file_flags_config` (whether enable `O_DIRECT`)

* parameters not affect
    * alignment size

```cpp
    void EngineRace::Benchmark() {
        const size_t value_file_size = VALUE_SIZE * (size_t) KEY_VALUE_MAX_COUNT_PER_THREAD;
        vector<uint32_t> block_size_config = {4096};
        vector<uint32_t> alignment_size_config = {4096};
        vector<uint32_t> thread_num_config = {64};
        uint32_t flag_config_num = 1;
        vector<int> write_file_flags_config = {O_RDWR};
        vector<int> read_file_flags_config = {O_RDONLY};

        uint32_t count = 0;
        log_info("Close file..");
        for (uint32_t block_size : block_size_config) {
            uint32_t block_num = (uint32_t)(value_file_size / block_size);
            uint32_t* file_block_offset = new uint32_t[block_num];
            for (uint32_t i = 0; i < block_num; ++i) {
                file_block_offset[i] = i;
            }

            for (uint32_t alignment_size : alignment_size_config) {
                for (uint32_t thread_num : thread_num_config) {
                    for (uint32_t flag_config = 0; flag_config < flag_config_num; ++flag_config) {
                        int write_file_flags = write_file_flags_config[flag_config];
                        int read_file_flags = read_file_flags_config[flag_config];

                        log_info("%d", count++);

                        TestDevice(write_file_flags, file_block_offset, block_num,
                                   read_file_flags, file_block_offset, block_num, thread_num, block_size, alignment_size);
                    }
                }
            }
            delete[] file_block_offset;
        }

        log_info("Close file end..");
        count = 0;
        for (uint32_t block_size : block_size_config) {
            uint32_t block_num = (uint32_t)(value_file_size / block_size);
            uint32_t* file_block_offset = new uint32_t[block_num];
            for (uint32_t i = 0; i < block_num; ++i) {
                file_block_offset[i] = i;
            }

            for (uint32_t shuffle_count = 0; shuffle_count < 3; ++shuffle_count) {
                auto seed = std::chrono::system_clock::now().time_since_epoch().count();

                shuffle(file_block_offset, file_block_offset + block_num, std::default_random_engine(seed));
            }

            for (uint32_t alignment_size : alignment_size_config) {
                for (uint32_t thread_num : thread_num_config) {
                    for (uint32_t flag_config = 0; flag_config < flag_config_num; ++flag_config) {
                        int write_file_flags = write_file_flags_config[flag_config];
                        int read_file_flags = read_file_flags_config[flag_config];

                        log_info("%d", count++);

                        TestDevice(write_file_flags, file_block_offset, block_num,
                                   read_file_flags, file_block_offset, block_num, thread_num, block_size, alignment_size);
                    }
                }
            }
            delete[] file_block_offset;
        }
    }
```

### Test Device Function

```cpp
    void EngineRace::TestDevice(int open_write_file_flag, uint32_t *write_file_block_offset, uint32_t write_block_num,
                                int open_read_file_flag, uint32_t *read_file_block_offset, uint32_t read_block_num,
                                uint32_t thread_num, uint32_t block_size, uint32_t alignment_size) {
        const string value_file_path = dir_ + "/" + value_file_name;

        write_value_file_dp_ = new int[thread_num];
        aligned_buffer_ = new char*[thread_num];

        for (uint32_t i = 0; i < thread_num; ++i) {
            string temp_value = value_file_path + to_string(i);

            write_value_file_dp_[i] = open(temp_value.c_str(), open_write_file_flag, FILE_PRIVILEGE);

            if (write_value_file_dp_[i] < 0) {
                log_info("Fail to open key-value files.");
                exit(-1);
            }

            aligned_buffer_[i] = (char *) memalign(alignment_size, block_size);
        }

        vector<thread> workers(thread_num);
        auto start = high_resolution_clock::now();

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i] = move(thread([write_file_block_offset, block_size, write_block_num, i, this]() {
                int local_file_dp = write_value_file_dp_[i];
                char* value_buffer = aligned_buffer_[i];

                for (uint32_t j = 0; j < write_block_num; ++j) {
                    size_t write_offset = (size_t)write_file_block_offset[j] * block_size;
                    pwrite(local_file_dp, value_buffer, block_size, write_offset);
                }
            }));
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i].join();
        }

        auto end = high_resolution_clock::now();
        log_info("Step one %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);

        for (uint32_t i = 0; i < thread_num; ++i) {
            fsync(write_value_file_dp_[i]);
            close(write_value_file_dp_[i]);
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            string temp_value = value_file_path + to_string(i);

            write_value_file_dp_[i] = open(temp_value.c_str(), open_read_file_flag, FILE_PRIVILEGE);

            if (write_value_file_dp_[i] < 0) {
                log_info("Fail to open key-value files.");
                exit(-1);
            }
        }

        start = high_resolution_clock::now();

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i] = move(thread([read_file_block_offset, block_size, read_block_num, i, this]() {
                int local_file_dp = write_value_file_dp_[i];
                char* value_buffer = aligned_buffer_[i];

                for (uint32_t j = 0; j < read_block_num; ++j) {
                    size_t read_offset = (size_t)read_file_block_offset[j] * block_size;
                    pread(local_file_dp, value_buffer, block_size, read_offset);
                }
            }));
        }

        for (uint32_t i = 0; i < thread_num; ++i) {
            workers[i].join();
        }

        end = high_resolution_clock::now();
        log_info("Step two %s, %s, %.3lf", strerror(errno),
                 FormatWithCommas(getValue()).c_str(), duration_cast<milliseconds>(end - start).count() / 1000.0);

        for (uint32_t i = 0; i < thread_num; ++i) {
            fsync(write_value_file_dp_[i]);
            close(write_value_file_dp_[i]);
            free(aligned_buffer_[i]);
        }

        delete[] aligned_buffer_;
        delete[] write_value_file_dp_;
    }
```

### 原理分析

* iostat, dstat sampling
* multi-files
* queue-depth

