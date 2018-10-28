## Update Logic

accumulate 1MB unmap, requiring `64MB + |P| * 1MB`

## Index

partition by range, i.e. `partition_id = key_int % |P|`

* in-memory: `|P|` sparesepp hash maps
* out-of-core: `|P|` : 
(1) count file, `4 |P|` bytes ; 
(2) idx files: index entries, each `12KB` flush once

## Data

4.5GB chunk per thread (for safety), without offset conflict

## Naming

name | content | truncated size | count of files
--- | --- | --- | ---
index-meta.redis | count file | 4KB | 1
index-`xxx`.redis | index file | truncate-dynamically | partition-num
values.redis | data file | `288GB =  4.5GB * 64`| 1
