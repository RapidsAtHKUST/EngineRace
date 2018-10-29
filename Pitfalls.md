## Volatile

* Never Read/Write From/To Volatile Together

```cpp
//        mmap_value_id_range_arr_[tid].end_idx_ = tmp;      // why will this change beg_idx_ also?
//        log_info("%s", strerror(errno));


```



force sync

```cpp
            msync(mmap_index_entry_arr_[partition_slot], INDEX_CHUNK_MMAP_SIZE, MS_SYNC);

```