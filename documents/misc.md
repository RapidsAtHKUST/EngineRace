## Affinity

https://blogs.igalia.com/dpino/2015/10/15/multicore-architectures-and-cpu-affinity/

> If a process is running on a core which heavily interacts
with an I/O device belonging to different NUMA node,
performance degradation issues may appear.
NUMA considerably benefits from the data locality principle,
so devices and processes operating on the same data
should run within the same NUMA node.

## Opt-ReadRandom

* (Fine-Grained Symc: Shrink 50)

```
------------------------------------------------
readrandom
22:46:47 INFO  (ts: 1544366807.846035) engine_race.cc:89: sizeof 8, 8, 10
22:46:47 (Func: Open, Line: 228), (TS: 1544366807.846098 s)-(Elapsed: 0.000063 s), (Mem: 490.390625 MB)

22:46:47 (Func: EngineRace, Line: 111), (TS: 1544366807.849902 s)-(Elapsed: 0.003867 s), (Mem: 490.433594 MB)

22:46:47 (Func: EngineRace, Line: 191), (TS: 1544366807.853572 s)-(Elapsed: 0.007537 s), (Mem: 490.445312 MB)

22:46:47 (Func: EngineRace, Line: 214), (TS: 1544366807.853880 s)-(Elapsed: 0.007845 s), (Mem: 490.449219 MB)

22:46:47 INFO  (ts: 1544366807.853918) engine_race.cc:907: After Flush Files, time: 0.007 s
22:46:47 (Func: EngineRace, Line: 216), (TS: 1544366807.853930 s)-(Elapsed: 0.007896 s), (Mem: 490.699219 MB)

22:46:47 DEBUG (ts: 1544366807.863467) util.h:109:


22:46:48 (Func: EngineRace, Line: 220), (TS: 1544366808.063550 s)-(Elapsed: 0.217515 s), (Mem: 1103.257812 MB)

22:46:48 (Func: EngineRace, Line: 223), (TS: 1544366808.063641 s)-(Elapsed: 0.217606 s), (Mem: 1103.257812 MB)

22:46:48 (Func: Open, Line: 242), (TS: 1544366808.063678 s)-(Elapsed: 0.217643 s), (Mem: 1103.257812 MB)

DB path: [test_directory]
22:46:48 INFO  (ts: 1544366808.066770) engine_race.cc:447: Off: 0, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066846) engine_race.cc:447: Off: 9126805504, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066860) engine_race.cc:447: Off: 18253611008, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066870) engine_race.cc:447: Off: 27380416512, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066878) engine_race.cc:447: Off: 36507222016, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066885) engine_race.cc:447: Off: 45634027520, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066892) engine_race.cc:447: Off: 54760833024, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066899) engine_race.cc:447: Off: 63887638528, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066906) engine_race.cc:447: Off: 73014444032, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066914) engine_race.cc:447: Off: 82141249536, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066922) engine_race.cc:447: Off: 91268055040, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066931) engine_race.cc:447: Off: 100394860544, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066938) engine_race.cc:447: Off: 109521666048, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066951) engine_race.cc:447: Off: 118648471552, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066960) engine_race.cc:447: Off: 127775277056, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066968) engine_race.cc:447: Off: 136902082560, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066976) engine_race.cc:447: Off: 146028888064, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066984) engine_race.cc:447: Off: 155155693568, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066992) engine_race.cc:447: Off: 164282499072, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.066999) engine_race.cc:447: Off: 173409304576, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067008) engine_race.cc:447: Off: 182536110080, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067015) engine_race.cc:447: Off: 191662915584, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067022) engine_race.cc:447: Off: 200789721088, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067029) engine_race.cc:447: Off: 209916526592, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067037) engine_race.cc:447: Off: 219043332096, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067044) engine_race.cc:447: Off: 228170137600, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067053) engine_race.cc:447: Off: 237296943104, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067060) engine_race.cc:447: Off: 246423748608, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067083) engine_race.cc:447: Off: 255550554112, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067217) engine_race.cc:447: Off: 264677359616, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067230) engine_race.cc:447: Off: 273804165120, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067238) engine_race.cc:447: Off: 282930970624, Size Not Need : 9126805504
22:46:48 INFO  (ts: 1544366808.067556) engine_race.cc:475: not found in tid: 56

22:46:48 INFO  (ts: 1544366808.070272) engine_race.cc:475: not found in tid: 63

22:46:48 DEBUG (ts: 1544366808.862233) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.04     3.21    5.35   10.12     0.15     0.06    28.44     0.01    0.50    0.71    0.39   0.09   0.14
nvme0n1           0.00   694.95 30854.69 4978.11   189.53   126.36    18.05     0.58    0.01    0.02    0.27   0.00  17.64


22:46:49 DEBUG (ts: 1544366809.862257) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00  477.00    0.00     5.54     0.00    23.78     0.46    0.96    0.96    0.00   0.12   5.90
nvme0n1           0.00     0.00 474321.00    0.00  2297.15     0.00     9.92    26.76    0.05    0.05    0.00   0.00  99.60


22:46:49 DEBUG (ts: 1544366809.960168) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
[7l----system---- ----total-cpu-usage---- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:46:47|  6   4  87   3   0   0| 190M  127M|30.9k 4989 |47.6 30.0 28.4|8134M 16.9M  122M  244G|   0     0 | 140k  158k| 1792  20383
09-12 22:46:48|  7   6  71  16   0   0|2276M    0 | 527k    0 |46.4 30.1 28.4|8444M 17.0M  122M  243G|   0     0 | 656k 1079k| 1792  20361
09-12 22:46:49|  2   7  75  16   0   0|2290M   56k| 586k 2.00 |46.4 30.1 28.4|8465M 17.6M  122M  243G|   0     0 | 708k 1198k| 1792  20404

22:46:50 DEBUG (ts: 1544366810.862278) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    12.00  182.00    2.00     0.71     0.05     8.52     0.15    0.82    0.82    0.50   0.09   1.70
nvme0n1           0.00     0.00 586355.00    0.00  2290.45     0.00     8.00    27.31    0.05    0.05    0.00   0.00 101.00


22:46:51 DEBUG (ts: 1544366811.862286) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586276.00    1.00  2290.14     0.00     8.00    27.71    0.05    0.05    0.00   0.00 101.70


22:46:52 DEBUG (ts: 1544366812.862317) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    5.00  118.00     0.02     0.55     9.56     0.08    0.67    0.60    0.68   0.02   0.30
nvme0n1           0.00     0.00 586476.00    0.00  2290.92     0.00     8.00    27.23    0.05    0.05    0.00   0.00 101.60


22:46:53 DEBUG (ts: 1544366813.862318) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    3.00    0.00     0.01     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586238.00    0.00  2289.99     0.00     8.00    27.39    0.04    0.04    0.00   0.00 102.20


22:46:54 DEBUG (ts: 1544366814.862353) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00  166.00    2.00     2.44     0.05    30.29     0.26    1.54    1.55    0.00   0.18   3.00
nvme0n1           0.00   304.00 585848.00  103.00  2288.47     1.59     8.00    27.40    0.04    0.04    0.31   0.00 101.80


22:46:54 DEBUG (ts: 1544366814.960555) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:46:50|  2   7  75  17   0   0|2291M 4096B| 586k 1.00 |46.4 30.1 28.4|8486M 17.6M  122M  243G|   0     0 | 709k 1198k| 1792  20403
09-12 22:46:51|  2   7  74  17   0   0|2291M  568k| 586k  118 |46.4 30.1 28.4|8506M 17.6M  122M  243G|   0     0 | 707k 1198k| 1792  20405
09-12 22:46:52|  2   7  72  19   0   0|2289M 1628k| 586k  103 |46.4 30.1 28.4|8525M 17.6M  122M  243G|   0     0 | 702k 1197k| 1792  20407
09-12 22:46:53|  2   7  72  19   0   0|2293M   64k| 587k 4.00 |45.3 30.1 28.4|8545M 18.1M  124M  243G|   0     0 | 707k 1199k| 1792  20455
09-12 22:46:54|  2   7  72  19   0   0|2293M    0 | 587k    0 |45.3 30.1 28.4|8569M 18.2M  126M  243G|   0     0 | 706k 1199k| 1920  20492

22:46:55 DEBUG (ts: 1544366815.862363) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   97.00    0.00     2.21     0.00    46.68     0.09    0.91    0.91    0.00   0.21   2.00
nvme0n1           0.00     0.00 586562.00    0.00  2291.26     0.00     8.00    27.25    0.04    0.04    0.00   0.00 101.70


22:46:56 DEBUG (ts: 1544366816.862399) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    0.00    2.00     0.00     0.04    44.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586749.00    0.00  2291.99     0.00     8.00    27.22    0.04    0.04    0.00   0.00 101.20


22:46:57 DEBUG (ts: 1544366817.862376) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    3.00    6.00     0.01     0.02     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586426.00    0.00  2290.73     0.00     8.00    27.37    0.04    0.04    0.00   0.00 102.00


22:46:58 DEBUG (ts: 1544366818.862425) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00  168.00    1.00    10.19     0.00   123.50     0.18    1.07    1.08    0.00   0.33   5.50
nvme0n1           0.00     0.00 586420.00    0.00  2290.70     0.00     8.00    27.07    0.04    0.04    0.00   0.00 101.40


22:46:59 DEBUG (ts: 1544366819.862437) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    19.00   60.00    2.00     1.49     0.08    51.87     0.07    1.18    1.22    0.00   0.19   1.20
nvme0n1           0.00     4.00 586610.00    1.00  2291.45     0.02     8.00    27.36    0.04    0.04    0.00   0.00 102.50


22:46:59 DEBUG (ts: 1544366819.960577) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:46:55|  2   7  71  20   0   0|2292M   44k| 587k 2.00 |45.3 30.1 28.4|8587M 18.2M  126M  243G|   0     0 | 698k 1199k| 1920  20492
09-12 22:46:56|  2   7  71  20   0   0|2291M   24k| 586k 6.00 |45.3 30.1 28.4|8605M 18.2M  126M  243G|   0     0 | 697k 1198k| 1920  20492
09-12 22:46:57|  2   8  69  21   0   0|2302M   24k| 587k 2.00 |45.3 30.1 28.4|8631M 18.5M  136M  243G|   0     0 | 698k 1204k| 1792  20522
09-12 22:46:58|  2   8  68  22   0   0|2292M   84k| 586k 2.00 |44.3 30.1 28.4|8643M 18.6M  138M  243G|   0     0 | 687k 1200k| 1792  22150
09-12 22:46:59|  2   8  69  22   0   0|2292M    0 | 587k    0 |44.3 30.1 28.4|8662M 18.6M  138M  243G|   0     0 | 685k 1200k| 1792  22161

22:47:00 DEBUG (ts: 1544366820.862477) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586669.00    0.00  2291.67     0.00     8.00    27.24    0.04    0.04    0.00   0.00 102.40


22:47:01 DEBUG (ts: 1544366821.862504) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   50.00    1.00     0.53     0.00    21.49     0.05    0.92    0.94    0.00   0.18   0.90
nvme0n1           0.00     0.00 586687.00    0.00  2291.75     0.00     8.00    27.11    0.04    0.04    0.00   0.00 102.00


22:47:02 DEBUG (ts: 1544366822.862521) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    25.00    8.00    3.00     0.05     0.11    30.55     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586570.00    0.00  2291.29     0.00     8.00    27.28    0.04    0.04    0.00   0.00 102.30


22:47:03 DEBUG (ts: 1544366823.862544) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586550.00    0.00  2291.21     0.00     8.00    27.01    0.04    0.04    0.00   0.00 101.70


22:47:04 DEBUG (ts: 1544366824.862565) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586674.00    0.00  2291.70     0.00     8.00    27.20    0.04    0.04    0.00   0.00 102.50


22:47:04 DEBUG (ts: 1544366824.960590) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:00|  2   8  68  23   0   0|2292M 4096B| 587k 1.00 |44.3 30.1 28.4|8678M 18.8M  138M  243G|   0     0 | 685k 1200k| 1792  22177
09-12 22:47:01|  2   8  66  25   0   0|2291M  112k| 587k 3.00 |44.3 30.1 28.4|8696M 18.8M  138M  243G|   0     0 | 670k 1200k| 1920  22172
09-12 22:47:02|  2   8  66  25   0   0|2291M    0 | 587k    0 |44.3 30.1 28.4|8712M 18.8M  138M  243G|   0     0 | 670k 1200k| 1920  22172
09-12 22:47:03|  2   8  66  25   0   0|2292M    0 | 587k    0 |44.3 30.1 28.4|8727M 18.8M  138M  243G|   0     0 | 672k 1200k| 1920  22173
09-12 22:47:04|  2   8  66  25   0   0|2290M    0 | 586k    0 |43.3 30.2 28.4|8742M 18.8M  138M  243G|   0     0 | 671k 1199k| 1920  22173

22:47:05 DEBUG (ts: 1544366825.862522) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586295.00    0.00  2290.21     0.00     8.00    27.02    0.04    0.04    0.00   0.00 101.80


22:47:06 DEBUG (ts: 1544366826.862588) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    1.00    2.00     0.00     0.03    24.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586358.00    0.00  2290.46     0.00     8.00    26.93    0.04    0.04    0.00   0.00 101.50


22:47:07 DEBUG (ts: 1544366827.862686) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     8.00    0.00   23.00     0.00     0.12    10.78     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586402.00    0.00  2290.64     0.00     8.00    26.99    0.04    0.04    0.00   0.00 101.60


22:47:08 DEBUG (ts: 1544366828.862705) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586489.00    0.00  2290.97     0.00     8.00    26.94    0.04    0.04    0.00   0.00 102.20


22:47:09 DEBUG (ts: 1544366829.862677) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    5.00    0.00     0.06     0.00    25.60     0.00    0.40    0.40    0.00   0.20   0.10
nvme0n1           0.00     0.00 586515.00    0.00  2291.07     0.00     8.00    27.22    0.04    0.04    0.00   0.00 102.40


22:47:09 DEBUG (ts: 1544366829.960593) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:05|  2   8  64  25   0   0|2290M   32k| 586k 2.00 |43.3 30.2 28.4|8758M 18.8M  138M  243G|   0     0 | 664k 1199k| 2048  22196
09-12 22:47:06|  2   8  65  26   0   0|2291M  124k| 587k 23.0 |43.3 30.2 28.4|8772M 18.8M  138M  243G|   0     0 | 665k 1200k| 2048  22174
09-12 22:47:07|  2   8  64  26   0   0|2290M    0 | 586k    0 |43.3 30.2 28.4|8786M 18.8M  138M  243G|   0     0 | 665k 1199k| 1920  22187
09-12 22:47:08|  2   8  65  26   0   0|2291M    0 | 587k    0 |43.3 30.2 28.4|8799M 18.8M  138M  243G|   0     0 | 667k 1200k| 1920  22178
09-12 22:47:09|  2   8  64  26   0   0|2292M 8192B| 586k 2.00 |43.3 30.2 28.4|8811M 18.8M  141M  243G|   0     0 | 661k 1199k| 1792  22184

22:47:10 DEBUG (ts: 1544366830.862606) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   37.00    2.00     2.95     0.01   155.08     0.02    0.46    0.49    0.00   0.41   1.60
nvme0n1           0.00     0.00 586171.00    0.00  2289.73     0.00     8.00    27.05    0.04    0.04    0.00   0.00 101.80


22:47:11 DEBUG (ts: 1544366831.862647) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    1.00    2.00     0.01     0.03    26.67     0.00    0.33    1.00    0.00   0.33   0.10
nvme0n1           0.00     0.00 586311.00    0.00  2290.27     0.00     8.00    26.79    0.04    0.04    0.00   0.00 102.00


22:47:12 DEBUG (ts: 1544366832.862689) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    0.00    2.00     0.00     0.04    44.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586645.00    0.00  2291.58     0.00     8.00    27.07    0.04    0.04    0.00   0.00 102.90


22:47:13 DEBUG (ts: 1544366833.862701) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586447.00    0.00  2290.81     0.00     8.00    27.09    0.04    0.04    0.00   0.00 103.30


22:47:14 DEBUG (ts: 1544366834.862726) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586586.00    0.00  2291.35     0.00     8.00    27.11    0.04    0.04    0.00   0.00 102.40


22:47:14 DEBUG (ts: 1544366834.960597) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:10|  2   8  64  26   0   0|2291M   32k| 586k 2.00 |43.3 30.2 28.4|8824M 18.8M  141M  243G|   0     0 | 661k 1199k| 1792  22196
09-12 22:47:11|  2   8  64  26   0   0|2291M   44k| 587k 2.00 |42.4 30.2 28.5|8836M 18.8M  141M  243G|   0     0 | 664k 1200k| 1792  22196
09-12 22:47:12|  2   8  64  26   0   0|2291M    0 | 586k    0 |42.4 30.2 28.5|8847M 18.8M  141M  243G|   0     0 | 660k 1199k| 1792  22185
09-12 22:47:13|  2   8  64  26   0   0|2291M    0 | 587k    0 |42.4 30.2 28.5|8859M 18.8M  141M  243G|   0     0 | 661k 1200k| 1792  22185
09-12 22:47:14|  2   8  64  26   0   0|2291M    0 | 587k    0 |42.4 30.2 28.5|8875M 18.8M  141M  243G|   0     0 | 662k 1200k| 1664  22186

22:47:15 DEBUG (ts: 1544366835.862735) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586574.00    0.00  2291.31     0.00     8.00    26.99    0.04    0.04    0.00   0.00 102.50


22:47:16 DEBUG (ts: 1544366836.862749) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586447.00    0.00  2290.81     0.00     8.00    27.30    0.04    0.04    0.00   0.00 102.50


22:47:17 DEBUG (ts: 1544366837.862659) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    17.00    2.00    7.00     0.01     0.09    23.11     0.00    0.11    0.50    0.00   0.11   0.10
nvme0n1           0.00     0.00 586334.00    0.00  2290.37     0.00     8.00    26.65    0.04    0.04    0.00   0.00 101.50


22:47:18 DEBUG (ts: 1544366838.862781) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586282.00    0.00  2290.16     0.00     8.00    27.46    0.04    0.04    0.00   0.00 103.40


22:47:19 DEBUG (ts: 1544366839.862808) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586494.00    0.00  2290.99     0.00     8.00    26.97    0.04    0.04    0.00   0.00 102.30


22:47:19 DEBUG (ts: 1544366839.960369) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:15|  2   8  64  26   0   0|2288M   32k| 586k 3.00 |42.4 30.2 28.5|8883M 18.9M  141M  243G|   0     0 | 661k 1198k| 1792  22202
09-12 22:47:16|  2   8  63  27   0   0|2292M   64k| 587k 4.00 |42.4 30.2 28.5|8897M 18.9M  141M  243G|   0     0 | 658k 1201k| 1792  22222
09-12 22:47:17|  2   8  63  27   0   0|2290M   84k| 586k 3.00 |41.5 30.2 28.5|8905M 18.9M  141M  243G|   0     0 | 654k 1200k| 1792  22238
09-12 22:47:18|  2   8  63  27   0   0|2291M    0 | 586k    0 |41.5 30.2 28.5|8915M 18.9M  141M  243G|   0     0 | 654k 1199k| 1792  22227
09-12 22:47:19|  2   9  61  28   0   0|2371M    0 | 587k    0 |41.5 30.2 28.5|8927M 19.2M  227M  243G|   0     0 | 658k 1204k| 2176  23840

22:47:20 DEBUG (ts: 1544366840.862825) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00 1064.00    0.00    80.60     0.00   155.14     0.70    0.66    0.66    0.00   0.49  51.80
nvme0n1           0.00     0.00 586400.00    0.00  2290.62     0.00     8.00    26.66    0.04    0.04    0.00   0.00 101.00


22:47:21 DEBUG (ts: 1544366841.862834) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    9.00    0.00     0.20     0.00    44.44     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586476.00    0.00  2290.93     0.00     8.00    27.27    0.04    0.04    0.00   0.00 102.70


22:47:22 DEBUG (ts: 1544366842.862847) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00   32.00   23.00     0.19     0.13    11.78     0.05    0.96    1.66    0.00   0.11   0.60
nvme0n1           0.00     0.00 586588.00    0.00  2291.36     0.00     8.00    27.13    0.04    0.04    0.00   0.00 102.60


22:47:23 DEBUG (ts: 1544366843.862826) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    2.00    2.00     0.02     0.05    32.00     0.00    0.50    1.00    0.00   0.50   0.20
nvme0n1           0.00     0.00 586356.00    0.00  2290.45     0.00     8.00    26.95    0.04    0.04    0.00   0.00 101.90


22:47:24 DEBUG (ts: 1544366844.862868) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586478.00    0.00  2290.93     0.00     8.00    27.10    0.04    0.04    0.00   0.00 102.50


22:47:24 DEBUG (ts: 1544366844.959668) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:20|  2   8  63  27   0   0|2292M    0 | 587k    0 |41.5 30.2 28.5|8941M 19.2M  227M  243G|   0     0 | 655k 1204k| 2176  23845
09-12 22:47:21|  2   8  63  27   0   0|2291M  132k| 586k 23.0 |41.5 30.2 28.5|8946M 19.3M  228M  243G|   0     0 | 655k 1204k| 2176  23849
09-12 22:47:22|  2   8  63  27   0   0|2289M   48k| 586k 2.00 |40.8 30.2 28.5|8961M 19.3M  228M  243G|   0     0 | 654k 1200k| 1920  23808
09-12 22:47:23|  2   8  63  27   0   0|2293M   20k| 587k 2.00 |40.8 30.2 28.5|8962M 19.3M  228M  243G|   0     0 | 656k 1201k| 1920  23819
09-12 22:47:24|  2   8  63  27   0   0|2289M    0 | 586k    0 |40.8 30.2 28.5|8967M 19.3M  228M  243G|   0     0 | 651k 1199k| 1920  23808

22:47:25 DEBUG (ts: 1544366845.862896) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586638.00    0.00  2291.56     0.00     8.00    26.82    0.04    0.04    0.00   0.00 102.20


22:47:26 DEBUG (ts: 1544366846.862900) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586589.00    0.00  2291.36     0.00     8.00    27.02    0.04    0.04    0.00   0.00 101.20


22:47:27 DEBUG (ts: 1544366847.862914) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     2.00    2.00  175.00     0.01     0.86     9.99     0.15    0.86    0.50    0.87   0.02   0.30
nvme0n1           0.00     0.00 586478.00    0.00  2290.93     0.00     8.00    27.12    0.04    0.04    0.00   0.00 102.60


22:47:28 DEBUG (ts: 1544366848.862927) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    14.00  128.00    2.00     7.45     0.06   118.34     0.09    0.66    0.67    0.00   0.34   4.40
nvme0n1           0.00     0.00 586517.00    0.00  2291.08     0.00     8.00    26.78    0.04    0.04    0.00   0.00 101.80


22:47:29 DEBUG (ts: 1544366849.862916) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    0.00    2.00     0.00     0.04    36.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586402.00    0.00  2290.63     0.00     8.00    26.69    0.04    0.04    0.00   0.00 102.00


22:47:29 DEBUG (ts: 1544366849.960465) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:25|  2   8  63  27   0   0|2294M    0 | 587k    0 |40.8 30.2 28.5|8975M 19.3M  228M  243G|   0     0 | 654k 1201k| 1920  23808
09-12 22:47:26|  2   8  63  27   0   0|2290M  876k| 586k  175 |40.8 30.2 28.5|8982M 19.3M  228M  243G|   0     0 | 651k 1199k| 1920  23810
09-12 22:47:27|  2   8  62  27   0   0|2297M   80k| 586k 6.00 |40.8 30.2 28.5|8986M 19.5M  235M  243G|   0     0 | 653k 1200k| 1920  24612
09-12 22:47:28|  2   8  62  27   0   0|2292M   36k| 587k 2.00 |40.1 30.3 28.5|8991M 19.5M  235M  243G|   0     0 | 653k 1201k| 1920  24624
09-12 22:47:29|  2   8  62  28   0   0|2290M    0 | 586k    0 |40.1 30.3 28.5|8997M 19.5M  235M  243G|   0     0 | 650k 1200k| 1920  24624

22:47:30 DEBUG (ts: 1544366850.862963) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586308.00    0.00  2290.26     0.00     8.00    27.12    0.04    0.04    0.00   0.00 102.40


22:47:31 DEBUG (ts: 1544366851.862995) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586651.00    0.00  2291.61     0.00     8.00    26.78    0.04    0.04    0.00   0.00 102.70


22:47:32 DEBUG (ts: 1544366852.863011) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    4.00     0.00     0.02     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586580.00    0.00  2291.33     0.00     8.00    26.99    0.04    0.04    0.00   0.00 102.80


22:47:33 DEBUG (ts: 1544366853.863038) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   32.00    0.00     0.12     0.00     8.00     0.06    2.00    2.00    0.00   0.09   0.30
nvme0n1           0.00     0.00 586743.00    0.00  2291.96     0.00     8.00    26.89    0.04    0.04    0.00   0.00 102.30


22:47:34 DEBUG (ts: 1544366854.863056) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586459.00    0.00  2290.86     0.00     8.00    26.84    0.04    0.04    0.00   0.00 102.50


22:47:34 DEBUG (ts: 1544366854.960537) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:30|  2   8  62  28   0   0|2292M 4096B| 587k 1.00 |40.1 30.3 28.5|8998M 19.5M  235M  243G|   0     0 | 658k 1201k| 1920  24613
09-12 22:47:31|  2   8  62  28   0   0|2291M   16k| 587k 4.00 |40.1 30.3 28.5|9001M 19.5M  235M  243G|   0     0 | 661k 1200k| 1920  24613
09-12 22:47:32|  2   8  62  28   0   0|2292M    0 | 587k    0 |40.1 30.3 28.5|9005M 19.6M  235M  243G|   0     0 | 649k 1200k| 1920  24614
09-12 22:47:33|  2   8  62  28   0   0|2291M  152k| 586k 4.00 |39.4 30.3 28.5|9009M 19.6M  235M  243G|   0     0 | 646k 1200k| 1920  24614
09-12 22:47:34|  2   8  62  28   0   0|2291M    0 | 587k    0 |39.4 30.3 28.5|9012M 19.6M  235M  243G|   0     0 | 647k 1200k| 1920  24614

22:47:35 DEBUG (ts: 1544366855.863081) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    34.00    0.00    4.00     0.00     0.15    76.00     0.00    0.25    0.00    0.25   0.25   0.10
nvme0n1           0.00     0.00 586650.00    0.00  2291.61     0.00     8.00    27.13    0.04    0.04    0.00   0.00 102.90


22:47:36 DEBUG (ts: 1544366856.863089) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586555.00    0.00  2291.23     0.00     8.00    26.82    0.04    0.04    0.00   0.00 101.90


22:47:37 DEBUG (ts: 1544366857.863126) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    3.00     0.01     0.01    10.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586615.00    0.00  2291.47     0.00     8.00    26.70    0.04    0.04    0.00   0.00 101.60


22:47:38 DEBUG (ts: 1544366858.863143) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586355.00    0.00  2290.45     0.00     8.00    27.12    0.04    0.04    0.00   0.00 103.10


22:47:39 DEBUG (ts: 1544366859.863155) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586703.00    0.00  2291.80     0.00     8.00    27.07    0.04    0.04    0.00   0.00 102.70


22:47:39 DEBUG (ts: 1544366859.960531) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:35|  2   8  63  27   0   0|2291M    0 | 587k    0 |39.4 30.3 28.5|9016M 19.6M  235M  243G|   0     0 | 650k 1200k| 2176  24637
09-12 22:47:36|  2   8  61  29   0   0|2291M   12k| 587k 3.00 |38.8 30.3 28.6|9019M 19.6M  235M  243G|   0     0 | 660k 1200k| 2176  24615
09-12 22:47:37|  2   8  62  28   0   0|2290M    0 | 586k    0 |38.8 30.3 28.6|9020M 19.6M  235M  243G|   0     0 | 646k 1199k| 2176  24615
09-12 22:47:38|  2   8  61  28   0   0|2292M    0 | 587k    0 |38.8 30.3 28.6|9023M 19.6M  235M  243G|   0     0 | 644k 1200k| 2176  24615
09-12 22:47:39|  2   8  61  28   0   0|2291M   52k| 586k 3.00 |38.8 30.3 28.6|9038M 19.6M  235M  243G|   0     0 | 644k 1200k| 2176  24616

22:47:40 INFO  (ts: 1544366860.482654) engine_race.cc:421: Read Stat of tid 56, elapsed time: 52.636 s, ts: 1544366860.482 s
22:47:40 INFO  (ts: 1544366860.581349) engine_race.cc:421: Read Stat of tid 63, elapsed time: 52.735 s, ts: 1544366860.581 s
22:47:40 DEBUG (ts: 1544366860.863173) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    1.00    3.00     0.00     0.05    28.00     0.00    0.50    1.00    0.33   0.25   0.10
nvme0n1           0.00     0.00 586465.00    0.00  2290.88     0.00     8.00    27.26    0.04    0.04    0.00   0.00 103.00


22:47:41 DEBUG (ts: 1544366861.863189) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586317.00    0.00  2290.30     0.00     8.00    27.25    0.04    0.04    0.00   0.00 103.40


22:47:42 DEBUG (ts: 1544366862.863213) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    12.00    0.00   20.00     0.00     0.12    12.80     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586039.00    0.00  2289.21     0.00     8.00    26.82    0.04    0.04    0.00   0.00 102.00


22:47:43 DEBUG (ts: 1544366863.863218) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586767.00    0.00  2292.06     0.00     8.00    27.05    0.04    0.04    0.00   0.00 102.20


22:47:44 DEBUG (ts: 1544366864.863244) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586211.00    0.00  2289.89     0.00     8.00    27.27    0.04    0.04    0.00   0.00 102.40


22:47:44 DEBUG (ts: 1544366864.960605) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:40|  2   8  61  29   0   0|2288M   44k| 586k 2.00 |38.2 30.4 28.6|9038M 19.6M  235M  243G|   0     0 | 641k 1198k| 2176  24616
09-12 22:47:41|  2   8  61  29   0   0|2289M   84k| 586k 18.0 |38.2 30.4 28.6|9036M 19.6M  235M  243G|   0     0 | 639k 1198k| 2176  24631
09-12 22:47:42|  2   8  61  29   0   0|2294M    0 | 587k    0 |38.2 30.4 28.6|9034M 19.6M  235M  243G|   0     0 | 638k 1200k| 2176  24616
09-12 22:47:43|  2   8  61  29   0   0|2288M    0 | 586k    0 |38.2 30.4 28.6|9035M 19.6M  235M  243G|   0     0 | 637k 1197k| 2176  24616
09-12 22:47:44|  2   8  61  29   0   0|2293M   32k| 587k 2.00 |37.7 30.4 28.6|9034M 19.6M  235M  243G|   0     0 | 638k 1200k| 2304  24618

22:47:45 DEBUG (ts: 1544366865.863256) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    2.00    0.00     0.02     0.00    16.00     0.00    0.50    0.50    0.00   0.50   0.10
nvme0n1           0.00     0.00 586623.00    0.00  2291.49     0.00     8.00    27.20    0.04    0.04    0.00   0.00 102.30


22:47:46 DEBUG (ts: 1544366866.863283) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586569.00    0.00  2291.29     0.00     8.00    27.25    0.04    0.04    0.00   0.00 102.60


22:47:47 DEBUG (ts: 1544366867.863299) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    6.00     0.00     0.02     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586314.00    0.00  2290.29     0.00     8.00    27.13    0.04    0.04    0.00   0.00 102.10


22:47:48 DEBUG (ts: 1544366868.863318) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586527.00    0.00  2291.12     0.00     8.00    27.12    0.04    0.04    0.00   0.00 103.30


22:47:49 DEBUG (ts: 1544366869.863317) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586499.00    0.00  2291.02     0.00     8.00    26.94    0.04    0.04    0.00   0.00 102.00


22:47:49 DEBUG (ts: 1544366869.959693) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:45|  2   8  61  29   0   0|2292M 4096B| 587k 1.00 |37.7 30.4 28.6|9035M 19.6M  235M  243G|   0     0 | 638k 1199k| 2304  24618
09-12 22:47:46|  2   8  61  29   0   0|2288M   20k| 586k 5.00 |37.7 30.4 28.6|9035M 19.6M  235M  243G|   0     0 | 636k 1197k| 2304  24618
09-12 22:47:47|  2   8  61  29   0   0|2291M   28k| 587k 2.00 |37.7 30.4 28.6|9035M 19.6M  235M  243G|   0     0 | 637k 1198k| 2176  24642
09-12 22:47:48|  2   8  61  29   0   0|2293M    0 | 587k    0 |37.3 30.4 28.6|9034M 19.6M  235M  243G|   0     0 | 638k 1200k| 2176  24618
09-12 22:47:49|  2   8  61  29   0   0|2290M    0 | 586k    0 |37.3 30.4 28.6|9034M 19.6M  235M  243G|   0     0 | 635k 1198k| 2176  24618

22:47:50 DEBUG (ts: 1544366870.863361) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586606.00    0.00  2291.43     0.00     8.00    26.77    0.04    0.04    0.00   0.00 102.20


22:47:51 DEBUG (ts: 1544366871.863377) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586478.00    0.00  2290.93     0.00     8.00    27.08    0.04    0.04    0.00   0.00 102.30


22:47:52 DEBUG (ts: 1544366872.863379) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    11.00    0.00    3.00     0.00     0.05    37.33     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586424.00    0.00  2290.72     0.00     8.00    27.11    0.04    0.04    0.00   0.00 102.90


22:47:53 DEBUG (ts: 1544366873.863540) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586408.00    0.00  2290.66     0.00     8.00    27.22    0.04    0.04    0.00   0.00 102.90


22:47:54 DEBUG (ts: 1544366874.863421) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 586425.00    0.00  2290.72     0.00     8.00    26.93    0.04    0.04    0.00   0.00 102.50


22:47:54 DEBUG (ts: 1544366874.960542) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:50|  2   8  61  29   0   0|2292M    0 | 587k    0 |37.3 30.4 28.6|9034M 19.6M  235M  243G|   0     0 | 637k 1199k| 2176  24618
09-12 22:47:51|  2   8  61  29   0   0|2291M   56k| 586k 3.00 |37.3 30.4 28.6|9034M 19.6M  235M  243G|   0     0 | 636k 1198k| 2176  24618
09-12 22:47:52|  2   8  61  29   0   0|2291M    0 | 586k    0 |37.3 30.4 28.6|9034M 19.6M  235M  243G|   0     0 | 637k 1198k| 2176  24618
09-12 22:47:53|  2   8  61  29   0   0|2290M   28k| 586k 2.00 |36.7 30.4 28.6|9033M 19.7M  235M  243G|   0     0 | 637k 1198k| 2176  24618
09-12 22:47:54|  2   8  61  29   0   0|2292M    0 | 587k    0 |36.7 30.4 28.6|9046M 19.7M  235M  243G|   0     0 | 639k 1199k| 2176  24630

22:47:55 DEBUG (ts: 1544366875.863440) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586632.00    0.00  2291.53     0.00     8.00    26.69    0.04    0.04    0.00   0.00 101.80


22:47:56 DEBUG (ts: 1544366876.863435) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586603.00    0.00  2291.42     0.00     8.00    27.29    0.04    0.04    0.00   0.00 103.30


22:47:57 DEBUG (ts: 1544366877.863453) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     8.00    0.00   22.00     0.00     0.12    10.91     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586358.00    0.00  2290.46     0.00     8.00    27.02    0.04    0.04    0.00   0.00 102.10


22:47:58 DEBUG (ts: 1544366878.863476) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586247.00    0.00  2290.03     0.00     8.00    27.09    0.04    0.04    0.00   0.00 102.50


22:47:59 DEBUG (ts: 1544366879.863484) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586547.00    0.00  2291.20     0.00     8.00    26.80    0.04    0.04    0.00   0.00 101.90


22:47:59 DEBUG (ts: 1544366879.960557) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:47:55|  2   8  61  29   0   0|2291M    0 | 587k    0 |36.7 30.4 28.6|9050M 19.7M  235M  243G|   0     0 | 636k 1199k| 2048  24621
09-12 22:47:56|  2   8  61  29   0   0|2290M  120k| 586k 22.0 |36.7 30.4 28.6|9047M 19.7M  235M  243G|   0     0 | 636k 1198k| 2048  24632
09-12 22:47:57|  2   8  61  29   0   0|2289M    0 | 586k    0 |36.7 30.4 28.6|9047M 19.7M  235M  243G|   0     0 | 635k 1197k| 2048  24621
09-12 22:47:58|  2   8  61  29   0   0|2293M   28k| 587k 2.00 |36.3 30.4 28.6|9047M 19.7M  235M  243G|   0     0 | 637k 1199k| 2048  24621
09-12 22:47:59|  2   8  61  29   0   0|2291M    0 | 587k    0 |36.3 30.4 28.6|9047M 19.7M  235M  243G|   0     0 | 637k 1198k| 2048  24621

22:48:00 DEBUG (ts: 1544366880.863486) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586527.00    0.00  2291.12     0.00     8.00    27.16    0.04    0.04    0.00   0.00 103.00


22:48:01 DEBUG (ts: 1544366881.863526) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586350.00    0.00  2290.43     0.00     8.00    27.28    0.04    0.04    0.00   0.00 102.70


22:48:02 DEBUG (ts: 1544366882.863540) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00  116.00     0.00     0.57    10.07     0.06    0.47    0.00    0.47   0.01   0.10
nvme0n1           0.00     0.00 586194.00    0.00  2289.82     0.00     8.00    26.87    0.04    0.04    0.00   0.00 102.40


22:48:03 DEBUG (ts: 1544366883.863574) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    26.00    0.00    2.00     0.00     0.11   112.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586447.00    0.00  2290.81     0.00     8.00    27.20    0.04    0.04    0.00   0.00 102.50


22:48:04 DEBUG (ts: 1544366884.863571) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586457.00    0.00  2290.85     0.00     8.00    27.09    0.04    0.04    0.00   0.00 102.80


22:48:04 DEBUG (ts: 1544366884.960373) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:00|  2   8  61  29   0   0|2290M 4096B| 586k 1.00 |36.3 30.4 28.6|9039M 19.7M  235M  243G|   0     0 | 637k 1199k| 1920  24632
09-12 22:48:01|  2   8  61  29   0   0|2290M  584k| 586k  116 |36.3 30.4 28.6|9041M 19.7M  235M  243G|   0     0 | 637k 1198k| 1920  24621
09-12 22:48:02|  2   8  61  29   0   0|2291M  112k| 586k 2.00 |36.3 30.4 28.6|9062M 19.7M  235M  243G|   0     0 | 637k 1198k| 1920  24622
09-12 22:48:03|  2   8  61  29   0   0|2291M    0 | 587k    0 |36.3 30.4 28.6|9060M 19.7M  235M  243G|   0     0 | 637k 1199k| 1920  24622
09-12 22:48:04|  2   8  61  29   0   0|2292M    0 | 587k    0 |36.0 30.4 28.6|9060M 19.7M  235M  243G|   0     0 | 637k 1199k| 1920  24622

22:48:05 DEBUG (ts: 1544366885.863590) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586863.00    0.00  2292.43     0.00     8.00    27.05    0.04    0.04    0.00   0.00 102.50


22:48:06 DEBUG (ts: 1544366886.863605) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586769.00    0.00  2292.07     0.00     8.00    27.22    0.04    0.04    0.00   0.00 103.00


22:48:07 DEBUG (ts: 1544366887.863619) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    12.00    0.00    3.00     0.00     0.06    40.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586619.00    0.00  2291.48     0.00     8.00    27.07    0.04    0.04    0.00   0.00 102.60


22:48:08 DEBUG (ts: 1544366888.863633) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586617.00    0.00  2291.47     0.00     8.00    27.19    0.04    0.04    0.00   0.00 102.50


22:48:09 DEBUG (ts: 1544366889.863658) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     4.00    3.00    2.00     0.03     0.02    22.40     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586427.00    0.00  2290.73     0.00     8.00    26.59    0.04    0.04    0.00   0.00 101.70


22:48:09 DEBUG (ts: 1544366889.960601) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:05|  2   8  61  29   0   0|2293M   56k| 587k 2.00 |36.0 30.4 28.6|9111M 19.7M  235M  242G|   0     0 | 638k 1199k| 1920  24635
09-12 22:48:06|  2   8  61  29   0   0|2291M 4096B| 587k 1.00 |36.0 30.4 28.6|9054M 19.7M  235M  243G|   0     0 | 637k 1199k| 1792  24633
09-12 22:48:07|  2   8  61  29   0   0|2292M    0 | 587k    0 |36.0 30.4 28.6|9054M 19.7M  235M  243G|   0     0 | 637k 1199k| 1792  24622
09-12 22:48:08|  2   8  61  28   0   0|2291M   24k| 586k 2.00 |36.0 30.4 28.6|9051M 19.7M  235M  243G|   0     0 | 639k 1199k| 1920  25419
09-12 22:48:09|  2   8  62  28   0   0|2292M 4096B| 587k 1.00 |36.0 30.4 28.6|9051M 19.7M  235M  243G|   0     0 | 638k 1199k| 1920  25407

22:48:10 DEBUG (ts: 1544366890.863678) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586746.00    0.00  2291.98     0.00     8.00    27.54    0.04    0.04    0.00   0.00 103.30


22:48:11 DEBUG (ts: 1544366891.863703) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586227.00    0.00  2289.95     0.00     8.00    26.87    0.04    0.04    0.00   0.00 102.20


22:48:12 DEBUG (ts: 1544366892.863714) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    4.00     0.00     0.04    20.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586347.00    0.00  2290.41     0.00     8.00    27.16    0.04    0.04    0.00   0.00 102.60


22:48:13 DEBUG (ts: 1544366893.863725) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586634.00    0.00  2291.54     0.00     8.00    27.10    0.04    0.04    0.00   0.00 102.30


22:48:14 DEBUG (ts: 1544366894.863745) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    11.00    0.00    2.00     0.00     0.05    52.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 586302.00    0.00  2290.24     0.00     8.00    26.91    0.04    0.04    0.00   0.00 102.30


22:48:14 DEBUG (ts: 1544366894.960617) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:10|  2   8  62  28   0   0|2290M   32k| 586k 2.00 |36.0 30.4 28.6|9056M 19.7M  235M  243G|   0     0 | 643k 1198k| 2048  25419
09-12 22:48:11|  2   8  62  28   0   0|2291M 8192B| 586k 2.00 |35.6 30.5 28.7|9050M 19.7M  235M  243G|   0     0 | 652k 1198k| 2048  25407
09-12 22:48:12|  2   8  62  28   0   0|2291M    0 | 587k    0 |35.6 30.5 28.7|9042M 19.7M  235M  243G|   0     0 | 657k 1199k| 2048  25419
09-12 22:48:13|  2   8  62  28   0   0|2290M   52k| 586k 2.00 |35.6 30.5 28.7|9042M 19.7M  235M  243G|   0     0 | 659k 1198k| 2048  25408
09-12 22:48:14|  2   8  62  28   0   0|2291M    0 | 587k    0 |35.6 30.5 28.7|9042M 19.7M  235M  243G|   0     0 | 647k 1198k| 2048  25408

22:48:15 DEBUG (ts: 1544366895.863763) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586477.00    0.00  2290.93     0.00     8.00    26.78    0.04    0.04    0.00   0.00 102.10


22:48:16 DEBUG (ts: 1544366896.863787) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586564.00    0.00  2291.27     0.00     8.00    27.27    0.04    0.04    0.00   0.00 103.60


22:48:17 DEBUG (ts: 1544366897.863815) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    3.00     0.00     0.03    21.33     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586400.00    0.00  2290.62     0.00     8.00    27.20    0.04    0.04    0.00   0.00 102.00


22:48:18 DEBUG (ts: 1544366898.863813) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586599.00    0.00  2291.41     0.00     8.00    27.02    0.04    0.04    0.00   0.00 102.20


22:48:19 DEBUG (ts: 1544366899.863840) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     8.00    0.00    2.00     0.00     0.04    40.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586519.00    0.00  2291.09     0.00     8.00    27.14    0.04    0.04    0.00   0.00 102.00


22:48:19 DEBUG (ts: 1544366899.960547) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:15|  2   8  62  28   0   0|2289M 4096B| 586k 1.00 |35.6 30.5 28.7|9042M 19.7M  235M  243G|   0     0 | 637k 1197k| 2048  25408
09-12 22:48:16|  2   8  62  28   0   0|2293M   28k| 587k 2.00 |35.6 30.5 28.7|9042M 19.7M  235M  243G|   0     0 | 639k 1199k| 2048  25408
09-12 22:48:17|  2   8  62  28   0   0|2289M    0 | 586k    0 |35.4 30.5 28.7|9048M 19.7M  235M  243G|   0     0 | 638k 1198k| 2048  25409
09-12 22:48:18|  2   8  62  28   0   0|2293M   40k| 587k 2.00 |35.4 30.5 28.7|9045M 19.7M  236M  243G|   0     0 | 640k 1199k| 1920  25420
09-12 22:48:19|  2   8  62  28   0   0|2295M    0 | 587k    0 |35.4 30.5 28.7|9050M 20.1M  239M  243G|   0     0 | 639k 1199k| 1920  25561

22:48:20 DEBUG (ts: 1544366900.863836) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00  210.00    0.00     3.79     0.00    36.91     0.09    0.43    0.43    0.00   0.18   3.70
nvme0n1           0.00     0.00 586379.00    0.00  2290.54     0.00     8.00    27.08    0.04    0.04    0.00   0.00 103.00


22:48:21 DEBUG (ts: 1544366901.863873) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586736.00    0.00  2291.94     0.00     8.00    27.13    0.04    0.04    0.00   0.00 102.20


22:48:22 DEBUG (ts: 1544366902.863881) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    8.00     0.00     0.03     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586302.00    0.00  2290.24     0.00     8.00    26.70    0.04    0.04    0.00   0.00 101.80


22:48:23 DEBUG (ts: 1544366903.863896) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586616.00    0.00  2291.47     0.00     8.00    27.22    0.04    0.04    0.00   0.00 102.60


22:48:24 DEBUG (ts: 1544366904.863915) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586279.00    0.00  2290.15     0.00     8.00    27.13    0.04    0.04    0.00   0.00 102.40


22:48:24 DEBUG (ts: 1544366904.960546) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:20|  2   8  62  28   0   0|2290M    0 | 586k    0 |35.4 30.5 28.7|9030M 20.1M  239M  243G|   0     0 | 640k 1200k| 1792  25539
09-12 22:48:21|  2   8  62  28   0   0|2292M   32k| 587k 8.00 |35.4 30.5 28.7|9031M 20.1M  239M  243G|   0     0 | 638k 1199k| 1792  25539
09-12 22:48:22|  2   8  62  28   0   0|2289M    0 | 586k    0 |35.1 30.5 28.7|9031M 20.1M  239M  243G|   0     0 | 639k 1197k| 1792  25539
09-12 22:48:23|  2   8  62  28   0   0|2292M   60k| 587k 4.00 |35.1 30.5 28.7|9031M 20.1M  239M  243G|   0     0 | 640k 1199k| 1792  25539
09-12 22:48:24|  2   8  62  27   0   0|2291M    0 | 587k    0 |35.1 30.5 28.7|9031M 20.1M  239M  243G|   0     0 | 640k 1199k| 1792  25550

22:48:25 DEBUG (ts: 1544366905.863921) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586644.00    0.00  2291.58     0.00     8.00    27.21    0.04    0.04    0.00   0.00 102.10


22:48:25 DEBUG (ts: 1544366905.960550) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:25|  2   8  62  27   0   0|2292M    0 | 587k    0 |35.1 30.5 28.7|9031M 20.1M  239M  243G|   0     0 | 642k 1198k| 1792  25539

22:48:26 DEBUG (ts: 1544366906.863950) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586659.00    0.00  2291.64     0.00     8.00    26.99    0.04    0.04    0.00   0.00 102.20


22:48:26 DEBUG (ts: 1544366906.960540) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:26|  2   8  63  27   0   0|2292M   88k| 587k 20.0 |35.1 30.5 28.7|9031M 20.1M  239M  243G|   0     0 | 642k 1199k| 1792  25539

22:48:27 DEBUG (ts: 1544366907.863967) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     2.00    1.00   20.00     0.00     0.09     8.76     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586676.00    0.00  2291.70     0.00     8.00    26.97    0.04    0.04    0.00   0.00 102.30


22:48:27 DEBUG (ts: 1544366907.960524) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:27|  2   8  63  27   0   0|2291M    0 | 586k    0 |35.1 30.5 28.7|9031M 20.1M  239M  243G|   0     0 | 640k 1198k| 1792  25539

22:48:28 DEBUG (ts: 1544366908.863989) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586424.00    0.00  2290.71     0.00     8.00    27.26    0.04    0.04    0.00   0.00 103.10


22:48:28 DEBUG (ts: 1544366908.959473) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:28|  2   8  63  27   0   0|2289M   36k| 586k 2.00 |34.8 30.6 28.7|9033M 20.1M  239M  243G|   0     0 | 639k 1197k| 1792  25552

22:48:29 DEBUG (ts: 1544366909.864024) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    0.00    2.00     0.00     0.04    36.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586664.00    0.00  2291.66     0.00     8.00    26.92    0.04    0.04    0.00   0.00 102.00


22:48:29 DEBUG (ts: 1544366909.960541) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:29|  2   8  63  27   0   0|2292M   28k| 587k 2.00 |34.8 30.6 28.7|9032M 20.1M  239M  243G|   0     0 | 643k 1199k| 1792  25539

22:48:30 DEBUG (ts: 1544366910.863993) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586294.00    0.00  2290.21     0.00     8.00    26.96    0.04    0.04    0.00   0.00 102.60


22:48:30 DEBUG (ts: 1544366910.960603) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:30|  2   8  63  27   0   0|2291M 4096B| 587k 1.00 |34.8 30.6 28.7|9033M 20.1M  239M  243G|   0     0 | 645k 1200k| 1920  25550

22:48:31 DEBUG (ts: 1544366911.864040) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586471.00    0.00  2290.91     0.00     8.00    27.01    0.04    0.04    0.00   0.00 102.10


22:48:31 DEBUG (ts: 1544366911.960602) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:31|  2   8  63  27   0   0|2290M    0 | 586k    0 |34.8 30.6 28.7|9033M 20.1M  239M  243G|   0     0 | 640k 1198k| 1920  25530

22:48:32 DEBUG (ts: 1544366912.959744) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
09-12 22:48:32|  2   8  63  27   0   0|2289M    0 | 586k    0 |34.8 30.6 28.7|9033M 20.1M  239M  243G|   0     0 | 638k 1197k| 1920  25530

22:48:33 INFO  (ts: 1544366913.574592) engine_race.cc:421: Read Stat of tid 17, elapsed time: 105.728 s, ts: 1544366913.574 s
22:48:33 INFO  (ts: 1544366913.589177) engine_race.cc:421: Read Stat of tid 16, elapsed time: 105.743 s, ts: 1544366913.589 s
22:48:33 INFO  (ts: 1544366913.596426) engine_race.cc:421: Read Stat of tid 7, elapsed time: 105.750 s, ts: 1544366913.596 s
22:48:33 INFO  (ts: 1544366913.621490) engine_race.cc:421: Read Stat of tid 49, elapsed time: 105.775 s, ts: 1544366913.621 s
22:48:33 INFO  (ts: 1544366913.656308) engine_race.cc:421: Read Stat of tid 52, elapsed time: 105.810 s, ts: 1544366913.656 s
22:48:33 INFO  (ts: 1544366913.660316) engine_race.cc:421: Read Stat of tid 60, elapsed time: 105.814 s, ts: 1544366913.660 s
22:48:33 INFO  (ts: 1544366913.669856) engine_race.cc:421: Read Stat of tid 4, elapsed time: 105.823 s, ts: 1544366913.669 s
22:48:33 INFO  (ts: 1544366913.672924) engine_race.cc:421: Read Stat of tid 36, elapsed time: 105.826 s, ts: 1544366913.672 s
22:48:33 INFO  (ts: 1544366913.683674) engine_race.cc:421: Read Stat of tid 15, elapsed time: 105.837 s, ts: 1544366913.683 s
22:48:33 INFO  (ts: 1544366913.695652) engine_race.cc:421: Read Stat of tid 33, elapsed time: 105.849 s, ts: 1544366913.695 s
22:48:33 INFO  (ts: 1544366913.697510) engine_race.cc:421: Read Stat of tid 22, elapsed time: 105.851 s, ts: 1544366913.697 s
22:48:33 INFO  (ts: 1544366913.700211) engine_race.cc:421: Read Stat of tid 19, elapsed time: 105.854 s, ts: 1544366913.700 s
22:48:33 INFO  (ts: 1544366913.704208) engine_race.cc:421: Read Stat of tid 53, elapsed time: 105.858 s, ts: 1544366913.704 s
22:48:33 INFO  (ts: 1544366913.706008) engine_race.cc:421: Read Stat of tid 6, elapsed time: 105.859 s, ts: 1544366913.706 s
22:48:33 INFO  (ts: 1544366913.706462) engine_race.cc:421: Read Stat of tid 29, elapsed time: 105.860 s, ts: 1544366913.706 s
22:48:33 INFO  (ts: 1544366913.708158) engine_race.cc:421: Read Stat of tid 20, elapsed time: 105.862 s, ts: 1544366913.708 s
22:48:33 INFO  (ts: 1544366913.708163) engine_race.cc:421: Read Stat of tid 32, elapsed time: 105.862 s, ts: 1544366913.708 s
22:48:33 INFO  (ts: 1544366913.710528) engine_race.cc:421: Read Stat of tid 42, elapsed time: 105.864 s, ts: 1544366913.710 s
22:48:33 INFO  (ts: 1544366913.713522) engine_race.cc:421: Read Stat of tid 2, elapsed time: 105.867 s, ts: 1544366913.713 s
22:48:33 INFO  (ts: 1544366913.717731) engine_race.cc:421: Read Stat of tid 11, elapsed time: 105.871 s, ts: 1544366913.717 s
22:48:33 INFO  (ts: 1544366913.718809) engine_race.cc:421: Read Stat of tid 51, elapsed time: 105.872 s, ts: 1544366913.718 s
22:48:33 INFO  (ts: 1544366913.722182) engine_race.cc:421: Read Stat of tid 37, elapsed time: 105.876 s, ts: 1544366913.722 s
22:48:33 INFO  (ts: 1544366913.727587) engine_race.cc:421: Read Stat of tid 31, elapsed time: 105.881 s, ts: 1544366913.727 s
22:48:33 INFO  (ts: 1544366913.728506) engine_race.cc:421: Read Stat of tid 48, elapsed time: 105.882 s, ts: 1544366913.728 s
22:48:33 INFO  (ts: 1544366913.732969) engine_race.cc:421: Read Stat of tid 43, elapsed time: 105.886 s, ts: 1544366913.732 s
22:48:33 INFO  (ts: 1544366913.734764) engine_race.cc:421: Read Stat of tid 44, elapsed time: 105.888 s, ts: 1544366913.734 s
22:48:33 INFO  (ts: 1544366913.736220) engine_race.cc:421: Read Stat of tid 8, elapsed time: 105.890 s, ts: 1544366913.736 s
22:48:33 INFO  (ts: 1544366913.736341) engine_race.cc:421: Read Stat of tid 45, elapsed time: 105.890 s, ts: 1544366913.736 s
22:48:33 INFO  (ts: 1544366913.737890) engine_race.cc:421: Read Stat of tid 18, elapsed time: 105.891 s, ts: 1544366913.737 s
22:48:33 INFO  (ts: 1544366913.738116) engine_race.cc:421: Read Stat of tid 35, elapsed time: 105.892 s, ts: 1544366913.738 s
22:48:33 INFO  (ts: 1544366913.738950) engine_race.cc:421: Read Stat of tid 12, elapsed time: 105.892 s, ts: 1544366913.738 s
22:48:33 INFO  (ts: 1544366913.742431) engine_race.cc:421: Read Stat of tid 34, elapsed time: 105.896 s, ts: 1544366913.742 s
22:48:33 INFO  (ts: 1544366913.742465) engine_race.cc:421: Read Stat of tid 5, elapsed time: 105.896 s, ts: 1544366913.742 s
22:48:33 INFO  (ts: 1544366913.743129) engine_race.cc:421: Read Stat of tid 59, elapsed time: 105.897 s, ts: 1544366913.743 s
22:48:33 INFO  (ts: 1544366913.744426) engine_race.cc:421: Read Stat of tid 13, elapsed time: 105.898 s, ts: 1544366913.744 s
22:48:33 INFO  (ts: 1544366913.745939) engine_race.cc:421: Read Stat of tid 38, elapsed time: 105.899 s, ts: 1544366913.745 s
22:48:33 INFO  (ts: 1544366913.746812) engine_race.cc:421: Read Stat of tid 50, elapsed time: 105.900 s, ts: 1544366913.746 s
22:48:33 INFO  (ts: 1544366913.748333) engine_race.cc:421: Read Stat of tid 40, elapsed time: 105.902 s, ts: 1544366913.748 s
22:48:33 INFO  (ts: 1544366913.748475) engine_race.cc:421: Read Stat of tid 9, elapsed time: 105.902 s, ts: 1544366913.748 s
22:48:33 INFO  (ts: 1544366913.754277) engine_race.cc:421: Read Stat of tid 27, elapsed time: 105.908 s, ts: 1544366913.754 s
22:48:33 INFO  (ts: 1544366913.754976) engine_race.cc:421: Read Stat of tid 39, elapsed time: 105.908 s, ts: 1544366913.754 s
22:48:33 INFO  (ts: 1544366913.755950) engine_race.cc:421: Read Stat of tid 28, elapsed time: 105.909 s, ts: 1544366913.755 s
22:48:33 INFO  (ts: 1544366913.758004) engine_race.cc:421: Read Stat of tid 47, elapsed time: 105.911 s, ts: 1544366913.758 s
22:48:33 INFO  (ts: 1544366913.759131) engine_race.cc:421: Read Stat of tid 3, elapsed time: 105.913 s, ts: 1544366913.759 s
22:48:33 INFO  (ts: 1544366913.760068) engine_race.cc:421: Read Stat of tid 26, elapsed time: 105.914 s, ts: 1544366913.760 s
22:48:33 INFO  (ts: 1544366913.762081) engine_race.cc:421: Read Stat of tid 62, elapsed time: 105.916 s, ts: 1544366913.762 s
22:48:33 INFO  (ts: 1544366913.766203) engine_race.cc:421: Read Stat of tid 41, elapsed time: 105.920 s, ts: 1544366913.766 s
22:48:33 INFO  (ts: 1544366913.766996) engine_race.cc:421: Read Stat of tid 25, elapsed time: 105.920 s, ts: 1544366913.766 s
22:48:33 INFO  (ts: 1544366913.771589) engine_race.cc:421: Read Stat of tid 54, elapsed time: 105.925 s, ts: 1544366913.771 s
22:48:33 INFO  (ts: 1544366913.775400) engine_race.cc:421: Read Stat of tid 61, elapsed time: 105.929 s, ts: 1544366913.775 s
22:48:33 INFO  (ts: 1544366913.776330) engine_race.cc:421: Read Stat of tid 55, elapsed time: 105.930 s, ts: 1544366913.776 s
22:48:33 INFO  (ts: 1544366913.779244) engine_race.cc:421: Read Stat of tid 30, elapsed time: 105.933 s, ts: 1544366913.779 s
22:48:33 INFO  (ts: 1544366913.779835) engine_race.cc:421: Read Stat of tid 21, elapsed time: 105.933 s, ts: 1544366913.779 s
22:48:33 INFO  (ts: 1544366913.788167) engine_race.cc:421: Read Stat of tid 23, elapsed time: 105.942 s, ts: 1544366913.788 s
22:48:33 INFO  (ts: 1544366913.792718) engine_race.cc:421: Read Stat of tid 0, elapsed time: 105.946 s, ts: 1544366913.792 s
22:48:33 INFO  (ts: 1544366913.795181) engine_race.cc:421: Read Stat of tid 58, elapsed time: 105.949 s, ts: 1544366913.795 s
22:48:33 INFO  (ts: 1544366913.797760) engine_race.cc:421: Read Stat of tid 46, elapsed time: 105.951 s, ts: 1544366913.797 s
22:48:33 INFO  (ts: 1544366913.798473) engine_race.cc:421: Read Stat of tid 1, elapsed time: 105.952 s, ts: 1544366913.798 s
22:48:33 INFO  (ts: 1544366913.800179) engine_race.cc:421: Read Stat of tid 24, elapsed time: 105.954 s, ts: 1544366913.800 s
22:48:33 INFO  (ts: 1544366913.807266) engine_race.cc:421: Read Stat of tid 10, elapsed time: 105.961 s, ts: 1544366913.807 s
22:48:33 INFO  (ts: 1544366913.821488) engine_race.cc:421: Read Stat of tid 14, elapsed time: 105.975 s, ts: 1544366913.821 s
22:48:33 INFO  (ts: 1544366913.822298) engine_race.cc:421: Read Stat of tid 57, elapsed time: 105.976 s, ts: 1544366913.822 s
readrandom   :       1.652 micros/op 605168 ops/sec; 2294.5 MB/s (1000000 of 1000000 found)

Microseconds per read:
Count: 64000000 Average: 104.0052  StdDev: 63.28
Min: 0  Median: 58.0004  Max: 10936
Percentiles: P50: 58.00 P75: 67.64 P99: 2383.08 P99.9: 2863.76 P99.99: 5866.59
------------------------------------------------------
[       0,       1 ]  1817212   2.839%   2.839% #
(       1,       2 ]    71195   0.111%   2.951%
(       2,       3 ]    30776   0.048%   2.999%
(       3,       4 ]    30429   0.048%   3.046%
(       4,       6 ]     9240   0.014%   3.061%
(       6,      10 ]     1148   0.002%   3.062%
(      10,      15 ]     1643   0.003%   3.065%
(      15,      22 ]    15108   0.024%   3.089%
(      22,      34 ]     8963   0.014%   3.103%
(      34,      51 ] 18395748  28.743%  31.846% ######
(      51,      76 ] 41492226  64.832%  96.678% #############
(      76,     110 ]   861763   1.347%  98.024%
(     110,     170 ]     4806   0.008%  98.032%
(     170,     250 ]      168   0.000%  98.032%
(     250,     380 ]       73   0.000%  98.032%
(     380,     580 ]       98   0.000%  98.032%
(     580,     870 ]      677   0.001%  98.033%
(     870,    1300 ]     4314   0.007%  98.040%
(    1300,    1900 ]    35524   0.056%  98.095%
(    1900,    2900 ]  1198319   1.872%  99.968%
(    2900,    4400 ]     1932   0.003%  99.971%
(    4400,    6600 ]    18358   0.029% 100.000%
(    6600,    9900 ]      279   0.000% 100.000%
(    9900,   14000 ]        1   0.000% 100.000%


------------------------------------------------
!!!Competion Report!!!
         readrandom: 605168 ops/second
disk usage: 278170 MB
------------------------------------------------
22:48:33 (Func: ~EngineRace, Line: 247), (TS: 1544366913.822745 s)-(Elapsed: 105.976710 s), (Mem: 1105.324219 MB)

22:48:33 (Func: ~EngineRace, Line: 310), (TS: 1544366913.822866 s)-(Elapsed: 105.976831 s), (Mem: 1105.539062 MB)

22:48:33 (Func: ~EngineRace, Line: 344), (TS: 1544366913.822934 s)-(Elapsed: 105.976899 s), (Mem: 1105.535156 MB)
```


* (Fine-Grained Symc: Shrink 10)

```
------------------------------------------------
readrandom
00:16:09 INFO  (ts: 1544372169.019552) engine_race.cc:89: sizeof 8, 8, 10
00:16:09 (Func: Open, Line: 228), (TS: 1544372169.019624 s)-(Elapsed: 0.000073 s), (Mem: 490.406250 MB)

00:16:09 (Func: EngineRace, Line: 111), (TS: 1544372169.019775 s)-(Elapsed: 0.000224 s), (Mem: 490.441406 MB)

00:16:09 (Func: EngineRace, Line: 191), (TS: 1544372169.027035 s)-(Elapsed: 0.007484 s), (Mem: 490.460938 MB)

00:16:09 (Func: EngineRace, Line: 214), (TS: 1544372169.027366 s)-(Elapsed: 0.007814 s), (Mem: 490.468750 MB)

00:16:09 INFO  (ts: 1544372169.027410) engine_race.cc:907: After Flush Files, time: 0.007 s
00:16:09 (Func: EngineRace, Line: 216), (TS: 1544372169.027423 s)-(Elapsed: 0.007872 s), (Mem: 490.718750 MB)

00:16:09 DEBUG (ts: 1544372169.030673) util.h:109:


00:16:09 (Func: EngineRace, Line: 220), (TS: 1544372169.234304 s)-(Elapsed: 0.214753 s), (Mem: 1103.152344 MB)

00:16:09 (Func: EngineRace, Line: 223), (TS: 1544372169.234401 s)-(Elapsed: 0.214849 s), (Mem: 1103.152344 MB)

00:16:09 (Func: Open, Line: 242), (TS: 1544372169.234438 s)-(Elapsed: 0.214887 s), (Mem: 1103.152344 MB)

DB path: [test_directory]
00:16:09 INFO  (ts: 1544372169.236406) engine_race.cc:447: Off: 0, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236524) engine_race.cc:447: Off: 9126805504, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236547) engine_race.cc:447: Off: 18253611008, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236557) engine_race.cc:447: Off: 27380416512, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236564) engine_race.cc:447: Off: 36507222016, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236571) engine_race.cc:447: Off: 45634027520, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236578) engine_race.cc:447: Off: 54760833024, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236585) engine_race.cc:447: Off: 63887638528, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236595) engine_race.cc:447: Off: 73014444032, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236602) engine_race.cc:447: Off: 82141249536, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236609) engine_race.cc:447: Off: 91268055040, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236617) engine_race.cc:447: Off: 100394860544, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236624) engine_race.cc:447: Off: 109521666048, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236630) engine_race.cc:447: Off: 118648471552, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236637) engine_race.cc:447: Off: 127775277056, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236645) engine_race.cc:447: Off: 136902082560, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236653) engine_race.cc:447: Off: 146028888064, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236668) engine_race.cc:447: Off: 155155693568, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236675) engine_race.cc:447: Off: 164282499072, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236683) engine_race.cc:447: Off: 173409304576, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236704) engine_race.cc:447: Off: 182536110080, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236714) engine_race.cc:447: Off: 191662915584, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236721) engine_race.cc:447: Off: 200789721088, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236727) engine_race.cc:447: Off: 209916526592, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236736) engine_race.cc:447: Off: 219043332096, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236743) engine_race.cc:447: Off: 228170137600, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236751) engine_race.cc:447: Off: 237296943104, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236758) engine_race.cc:447: Off: 246423748608, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236767) engine_race.cc:447: Off: 255550554112, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236774) engine_race.cc:447: Off: 264677359616, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236783) engine_race.cc:447: Off: 273804165120, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.236790) engine_race.cc:447: Off: 282930970624, Size Not Need : 9126805504
00:16:09 INFO  (ts: 1544372169.237145) engine_race.cc:475: not found in tid: 60

00:16:09 INFO  (ts: 1544372169.237154) engine_race.cc:475: not found in tid: 62

00:16:10 DEBUG (ts: 1544372170.030500) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.04     3.20    4.37    8.80     0.12     0.06    27.90     0.01    0.48    0.67    0.38   0.08   0.11
nvme0n1           0.00   656.11 26398.34 4461.41   163.40   110.99    18.21     0.44    0.01    0.00    0.06   0.01  15.55


00:16:11 DEBUG (ts: 1544372171.030500) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00  215.00    0.00     3.02     0.00    28.76     0.20    0.94    0.94    0.00   0.21   4.50
nvme0n1           0.00     0.00 473709.00    0.00  2308.54     0.00     9.98    25.60    0.05    0.05    0.00   0.00  99.70


00:16:11 DEBUG (ts: 1544372171.123946) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
[7l----system---- ----total-cpu-usage---- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:09|  5   4  89   2   0   0| 164M  112M|26.4k 4471 |47.8 32.4 28.4|8291M 13.7M  112M  243G|   0     0 | 124k  113k| 1792  18310
10-12 00:16:10|  9   7  70  14   0   0|2288M    0 | 524k    0 |46.5 32.4 28.4|8627M 13.7M  115M  243G|   0     0 | 746k 1155k| 1792  18310
10-12 00:16:11|  3   8  74  15   0   0|2290M   36k| 586k 2.00 |46.5 32.4 28.4|8654M 13.7M  115M  243G|   0     0 | 848k 1291k| 1792  18310

00:16:12 DEBUG (ts: 1544372172.030577) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    0.00    2.00     0.00     0.04    36.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586629.00    0.00  2291.52     0.00     8.00    26.16    0.04    0.04    0.00   0.00 101.10


00:16:13 DEBUG (ts: 1544372173.030670) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   33.00    0.00     0.13     0.00     8.00     0.05    1.52    1.52    0.00   0.06   0.20
nvme0n1           0.00     0.00 586619.00    0.00  2291.48     0.00     8.00    26.02    0.04    0.04    0.00   0.00 101.10


00:16:14 DEBUG (ts: 1544372174.030631) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    13.00    0.00  185.00     0.00     0.91    10.03     0.14    0.75    0.00    0.75   0.01   0.20
nvme0n1           0.00     0.00 586797.00    0.00  2292.18     0.00     8.00    26.04    0.04    0.04    0.00   0.00 101.50


00:16:15 DEBUG (ts: 1544372175.030567) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     1.00  582.00    3.00     6.97     0.02    24.46     0.38    0.64    0.64    0.00   0.14   8.30
nvme0n1           0.00   284.00 586458.00   11.00  2290.86     1.15     8.00    25.93    0.04    0.04    0.18   0.00 101.70


00:16:16 DEBUG (ts: 1544372176.030594) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00  440.00    0.00     9.43     0.00    43.87     0.33    0.75    0.75    0.00   0.19   8.40
nvme0n1           0.00     0.00 586903.00    0.00  2292.59     0.00     8.00    25.71    0.04    0.04    0.00   0.00 101.00


00:16:16 DEBUG (ts: 1544372176.124573) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:12|  3   8  75  14   0   0|2293M    0 | 587k    0 |46.5 32.4 28.4|8668M 13.9M  115M  243G|   0     0 | 836k 1292k| 1792  18313
10-12 00:16:13|  3   8  73  16   0   0|2291M  928k| 586k  185 |46.5 32.4 28.4|8693M 13.9M  115M  243G|   0     0 | 829k 1291k| 1792  18313
10-12 00:16:14|  3   8  73  16   0   0|2300M 1300k| 587k 20.0 |45.4 32.4 28.4|8713M 15.4M  120M  243G|   0     0 | 827k 1293k| 1920  18493
10-12 00:16:15|  3   9  70  18   0   0|2301M    0 | 587k    0 |45.4 32.4 28.4|8738M 16.5M  127M  243G|   0     0 | 808k 1295k| 2048  21707
10-12 00:16:16|  3   8  66  22   0   0|2293M    0 | 587k    0 |45.4 32.4 28.4|8754M 16.5M  129M  243G|   0     0 | 759k 1296k| 2048  21694

00:16:17 DEBUG (ts: 1544372177.030709) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586658.00    0.00  2291.63     0.00     8.00    26.12    0.04    0.04    0.00   0.00 101.50


00:16:18 DEBUG (ts: 1544372178.030721) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    1.00    2.00     0.01     0.04    34.67     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586753.00    0.00  2292.00     0.00     8.00    26.23    0.04    0.04    0.00   0.00 102.20


00:16:19 DEBUG (ts: 1544372179.030653) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    2.00    2.00     0.01     0.01     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586416.00    0.00  2290.69     0.00     8.00    26.42    0.04    0.04    0.00   0.00 102.10


00:16:20 DEBUG (ts: 1544372180.030679) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    9.00    0.00     0.20     0.00    44.44     0.00    0.22    0.22    0.00   0.22   0.20
nvme0n1           0.00   213.00 586275.00   80.00  2290.14     1.14     8.00    26.56    0.04    0.04    0.19   0.00 102.50


00:16:21 DEBUG (ts: 1544372181.030715) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    1.00    2.00     0.00     0.03    21.33     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586280.00    0.00  2290.15     0.00     8.00    26.32    0.04    0.04    0.00   0.00 103.10


00:16:21 DEBUG (ts: 1544372181.124549) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:17|  3   9  64  24   0   0|2292M   44k| 587k 2.00 |45.4 32.4 28.4|8774M 16.5M  129M  243G|   0     0 | 741k 1297k| 1920  21706
10-12 00:16:18|  3   9  64  24   0   0|2291M 8192B| 586k 2.00 |44.3 32.4 28.4|8794M 16.6M  129M  243G|   0     0 | 736k 1297k| 1920  21695
10-12 00:16:19|  3   9  64  25   0   0|2290M 1192k| 586k 82.0 |44.3 32.4 28.4|8812M 16.6M  129M  243G|   0     0 | 736k 1297k| 1920  22494
10-12 00:16:20|  3   9  63  25   0   0|2290M   28k| 586k 2.00 |44.3 32.4 28.4|8842M 16.6M  129M  243G|   0     0 | 736k 1297k| 1920  22483
10-12 00:16:21|  3   9  63  25   0   0|2294M    0 | 587k    0 |44.3 32.4 28.4|8857M 16.6M  131M  243G|   0     0 | 730k 1299k| 1664  22495

00:16:22 DEBUG (ts: 1544372182.030676) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   62.00    0.00     2.03     0.00    67.10     0.01    0.23    0.23    0.00   0.21   1.30
nvme0n1           0.00     0.00 586622.00    0.00  2291.49     0.00     8.00    26.46    0.04    0.04    0.00   0.00 102.40


00:16:23 DEBUG (ts: 1544372183.030760) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586468.00    0.00  2290.89     0.00     8.00    26.35    0.04    0.04    0.00   0.00 102.40


00:16:24 DEBUG (ts: 1544372184.030879) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    0.00   75.00     0.00     0.34     9.39     0.00    0.03    0.00    0.03   0.01   0.10
nvme0n1           0.00     0.00 586894.00    0.00  2292.55     0.00     8.00    26.25    0.04    0.04    0.00   0.00 102.50


00:16:25 DEBUG (ts: 1544372185.030871) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586457.00    0.00  2290.85     0.00     8.00    26.40    0.04    0.04    0.00   0.00 102.70


00:16:26 DEBUG (ts: 1544372186.030815) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586783.00    0.00  2292.12     0.00     8.00    26.45    0.04    0.04    0.00   0.00 102.80


00:16:26 DEBUG (ts: 1544372186.124557) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:22|  3   9  63  25   0   0|2291M    0 | 587k    0 |43.3 32.4 28.4|8874M 16.6M  131M  243G|   0     0 | 729k 1298k| 1664  22484
10-12 00:16:23|  3   9  63  25   0   0|2291M  352k| 586k 75.0 |43.3 32.4 28.4|8891M 16.6M  131M  243G|   0     0 | 729k 1298k| 1664  22481
10-12 00:16:24|  3   9  63  25   0   0|2292M   12k| 587k 3.00 |43.3 32.4 28.4|8908M 16.6M  131M  243G|   0     0 | 730k 1298k| 1664  22481
10-12 00:16:25|  3   9  63  25   0   0|2292M    0 | 587k    0 |43.3 32.4 28.4|8924M 16.6M  131M  243G|   0     0 | 730k 1298k| 1664  22481
10-12 00:16:26|  3   9  63  25   0   0|2291M   44k| 587k 2.00 |43.3 32.4 28.4|8940M 16.6M  131M  243G|   0     0 | 728k 1298k| 1664  22482

00:16:27 DEBUG (ts: 1544372187.030847) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    0.00    2.00     0.00     0.04    44.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586545.00    0.00  2291.19     0.00     8.00    26.20    0.04    0.04    0.00   0.00 102.60


00:16:28 DEBUG (ts: 1544372188.030847) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586501.00    0.00  2291.02     0.00     8.00    26.48    0.04    0.04    0.00   0.00 102.50


00:16:29 DEBUG (ts: 1544372189.030929) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    16.00   38.00   24.00     2.95     0.17   103.10     0.02    0.29    0.47    0.00   0.26   1.60
nvme0n1           0.00     0.00 586363.00    0.00  2290.48     0.00     8.00    25.99    0.04    0.04    0.00   0.00 102.20


00:16:30 DEBUG (ts: 1544372190.030976) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586809.00    0.00  2292.23     0.00     8.00    26.31    0.04    0.04    0.00   0.00 102.00


00:16:31 DEBUG (ts: 1544372191.030872) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586851.00    0.00  2292.38     0.00     8.00    26.14    0.04    0.04    0.00   0.00 101.60


00:16:31 DEBUG (ts: 1544372191.124557) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:27|  3   9  63  25   0   0|2291M    0 | 587k    0 |42.4 32.4 28.4|8957M 16.6M  131M  243G|   0     0 | 726k 1298k| 1664  22492
10-12 00:16:28|  3   9  62  26   0   0|2292M  176k| 586k 24.0 |42.4 32.4 28.4|9005M 16.6M  134M  243G|   0     0 | 723k 1298k| 1536  22486
10-12 00:16:29|  3   9  63  25   0   0|2293M    0 | 587k    0 |42.4 32.4 28.4|9008M 16.6M  134M  243G|   0     0 | 726k 1299k| 1536  22487
10-12 00:16:30|  3   9  63  26   0   0|2292M 4096B| 587k 1.00 |42.4 32.4 28.4|9022M 16.6M  134M  243G|   0     0 | 723k 1299k| 1536  22487
10-12 00:16:31|  3   9  62  25   0   0|2292M 4096B| 587k 1.00 |41.6 32.4 28.5|9016M 16.7M  134M  243G|   0     0 | 725k 1299k| 1792  22519

00:16:32 DEBUG (ts: 1544372192.030912) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   14.00    1.00     0.38     0.00    52.27     0.00    0.27    0.29    0.00   0.27   0.40
nvme0n1           0.00     0.00 586487.00    0.00  2290.96     0.00     8.00    25.85    0.04    0.04    0.00   0.00 101.70


00:16:33 DEBUG (ts: 1544372193.030935) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    5.00    2.00     0.05     0.03    22.86     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586610.00    0.00  2291.45     0.00     8.00    25.98    0.04    0.04    0.00   0.00 101.50


00:16:34 DEBUG (ts: 1544372194.031038) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    28.00    4.00   11.00     0.06     0.16    29.33     0.00    0.07    0.25    0.00   0.07   0.10
nvme0n1           0.00     0.00 586635.00    0.00  2291.55     0.00     8.00    26.35    0.04    0.04    0.00   0.00 102.10


00:16:35 DEBUG (ts: 1544372195.030964) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.01     0.00    16.00     0.00    1.00    1.00    0.00   1.00   0.10
nvme0n1           0.00     0.00 586787.00    0.00  2292.13     0.00     8.00    26.37    0.04    0.04    0.00   0.00 102.70


00:16:36 DEBUG (ts: 1544372196.031072) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586580.00    0.00  2291.33     0.00     8.00    26.22    0.04    0.04    0.00   0.00 103.00


00:16:36 DEBUG (ts: 1544372196.124623) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:32|  3   9  63  25   0   0|2290M   28k| 586k 2.00 |41.6 32.4 28.5|9029M 16.7M  135M  243G|   0     0 | 726k 1298k| 1920  22499
10-12 00:16:33|  3   9  63  25   0   0|2293M  160k| 587k 11.0 |41.6 32.4 28.5|9041M 16.7M  135M  243G|   0     0 | 726k 1299k| 1792  22513
10-12 00:16:34|  3   9  63  25   0   0|2290M    0 | 586k    0 |41.6 32.4 28.5|9053M 16.7M  135M  243G|   0     0 | 726k 1298k| 1792  22504
10-12 00:16:35|  3   9  63  25   0   0|2292M    0 | 587k    0 |41.6 32.4 28.5|9065M 16.7M  135M  243G|   0     0 | 725k 1298k| 1792  22504
10-12 00:16:36|  3   9  62  25   0   0|2293M    0 | 587k    0 |41.6 32.4 28.5|9082M 16.7M  135M  243G|   0     0 | 725k 1300k| 1664  22506

00:16:37 DEBUG (ts: 1544372197.031107) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    2.00    0.00     0.01     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586693.00    0.00  2291.77     0.00     8.00    26.31    0.04    0.04    0.00   0.00 103.00


00:16:38 DEBUG (ts: 1544372198.031035) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    0.00    2.00     0.00     0.05    48.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586780.00    1.00  2292.11     0.00     8.00    26.43    0.04    0.04    0.00   0.00 103.30


00:16:39 DEBUG (ts: 1544372199.031137) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586391.00    0.00  2290.59     0.00     8.00    26.32    0.04    0.04    0.00   0.00 102.70


00:16:40 DEBUG (ts: 1544372200.031110) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    4.00    2.00     0.02     0.03    16.00     0.00    0.17    0.00    0.50   0.17   0.10
nvme0n1           0.00     0.00 586784.00    0.00  2292.12     0.00     8.00    26.15    0.04    0.04    0.00   0.00 101.60


00:16:41 DEBUG (ts: 1544372201.031092) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586659.00    0.00  2291.64     0.00     8.00    26.39    0.04    0.04    0.00   0.00 102.60


00:16:41 DEBUG (ts: 1544372201.123967) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:37|  3   9  63  25   0   0|2292M   52k| 587k 3.00 |41.6 32.4 28.5|9093M 16.7M  135M  243G|   0     0 | 725k 1298k| 1664  22506
10-12 00:16:38|  3   9  62  26   0   0|2291M    0 | 586k    0 |40.6 32.3 28.5|9104M 16.7M  135M  243G|   0     0 | 724k 1298k| 1664  22506
10-12 00:16:39|  3   9  62  26   0   0|2291M   32k| 586k 2.00 |40.6 32.3 28.5|9115M 16.7M  135M  243G|   0     0 | 723k 1298k| 1664  22517
10-12 00:16:40|  3   9  62  26   0   0|2291M    0 | 587k    0 |40.6 32.3 28.5|9125M 16.7M  135M  243G|   0     0 | 724k 1298k| 1664  22506
10-12 00:16:41|  3   9  62  26   0   0|2291M    0 | 587k    0 |40.6 32.3 28.5|9135M 16.7M  135M  243G|   0     0 | 724k 1298k| 1664  22506

00:16:42 DEBUG (ts: 1544372202.031093) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586633.00    0.00  2291.53     0.00     8.00    26.30    0.04    0.04    0.00   0.00 102.40


00:16:43 DEBUG (ts: 1544372203.031158) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586859.00    0.00  2292.42     0.00     8.00    26.29    0.04    0.04    0.00   0.00 102.30


00:16:44 DEBUG (ts: 1544372204.031140) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    2.00     0.00     0.01     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586700.00    1.00  2291.79     0.00     8.00    26.41    0.04    0.04    0.00   0.00 101.90


00:16:45 DEBUG (ts: 1544372205.031221) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    0.00    4.00     0.00     0.04    22.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586521.00    0.00  2291.11     0.00     8.00    26.20    0.04    0.04    0.00   0.00 102.60


00:16:46 DEBUG (ts: 1544372206.031204) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 587195.00    0.00  2293.73     0.00     8.00    26.23    0.04    0.04    0.00   0.00 103.00


00:16:46 DEBUG (ts: 1544372206.123962) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:42|  3   9  62  26   0   0|2294M    0 | 587k    0 |40.6 32.3 28.5|9144M 16.7M  135M  243G|   0     0 | 725k 1300k| 1664  22506
10-12 00:16:43|  3   9  62  26   0   0|2292M   12k| 587k 3.00 |40.6 32.3 28.5|9153M 16.7M  135M  243G|   0     0 | 724k 1298k| 1664  22506
10-12 00:16:44|  3   9  62  26   0   0|2290M   44k| 586k 4.00 |40.0 32.3 28.5|9161M 16.7M  135M  243G|   0     0 | 724k 1297k| 1664  22506
10-12 00:16:45|  3   9  62  25   0   0|2294M    0 | 587k    0 |40.0 32.3 28.5|9167M 16.7M  135M  243G|   0     0 | 725k 1300k| 1664  22517
10-12 00:16:46|  3   9  62  26   0   0|2291M    0 | 587k    0 |40.0 32.3 28.5|9175M 16.7M  135M  243G|   0     0 | 723k 1298k| 1664  22506

00:16:47 DEBUG (ts: 1544372207.031164) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586560.00    0.00  2291.24     0.00     8.00    26.31    0.04    0.04    0.00   0.00 102.60


00:16:48 DEBUG (ts: 1544372208.031227) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586739.00    0.00  2291.96     0.00     8.00    26.26    0.04    0.04    0.00   0.00 102.60


00:16:49 DEBUG (ts: 1544372209.031214) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     4.00    0.00   19.00     0.00     0.12    12.63     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586609.00    0.00  2291.44     0.00     8.00    25.94    0.04    0.04    0.00   0.00 101.90


00:16:50 DEBUG (ts: 1544372210.031300) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.01     0.00    16.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586808.00    0.00  2292.22     0.00     8.00    26.18    0.04    0.04    0.00   0.00 102.70


00:16:51 DEBUG (ts: 1544372211.031291) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    14.00    0.00    4.00     0.00     0.07    36.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586750.00    0.00  2291.99     0.00     8.00    25.91    0.04    0.04    0.00   0.00 101.50


00:16:51 DEBUG (ts: 1544372211.123947) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:47|  3   9  62  26   0   0|2294M    0 | 587k    0 |40.0 32.3 28.5|9182M 16.7M  135M  242G|   0     0 | 723k 1299k| 1664  22506
10-12 00:16:48|  3   9  62  26   0   0|2290M  120k| 586k 19.0 |40.0 32.3 28.5|9189M 16.7M  135M  242G|   0     0 | 723k 1297k| 1664  22506
10-12 00:16:49|  3   9  62  26   0   0|2293M    0 | 587k    0 |40.0 32.3 28.5|9196M 16.7M  135M  242G|   0     0 | 725k 1299k| 1664  22518
10-12 00:16:50|  3   9  62  26   0   0|2292M   72k| 587k 4.00 |39.3 32.3 28.5|9201M 16.7M  135M  242G|   0     0 | 726k 1299k| 1664  22507
10-12 00:16:51|  3   9  62  26   0   0|2290M    0 | 586k    0 |39.3 32.3 28.5|9206M 16.7M  135M  242G|   0     0 | 740k 1298k| 1792  22518

00:16:52 DEBUG (ts: 1544372212.031306) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586626.00    0.00  2291.51     0.00     8.00    25.74    0.04    0.04    0.00   0.00 101.80


00:16:53 DEBUG (ts: 1544372213.031293) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586683.00    0.00  2291.73     0.00     8.00    26.07    0.04    0.04    0.00   0.00 102.20


00:16:54 DEBUG (ts: 1544372214.031438) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586549.00    0.00  2291.21     0.00     8.00    26.23    0.04    0.04    0.00   0.00 103.10


00:16:55 DEBUG (ts: 1544372215.031431) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586730.00    0.00  2291.91     0.00     8.00    25.81    0.04    0.04    0.00   0.00 102.40


00:16:56 DEBUG (ts: 1544372216.031358) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    4.00    0.00     0.02     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586655.00    0.00  2291.62     0.00     8.00    26.04    0.04    0.04    0.00   0.00 102.20


00:16:56 DEBUG (ts: 1544372216.124552) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:52|  3   9  62  26   0   0|2293M    0 | 587k    0 |39.3 32.3 28.5|9211M 16.7M  135M  242G|   0     0 | 753k 1299k| 1792  22507
10-12 00:16:53|  3   9  62  26   0   0|2291M 4096B| 587k 1.00 |39.3 32.3 28.5|9216M 16.7M  135M  242G|   0     0 | 745k 1298k| 1792  22507
10-12 00:16:54|  3   9  63  25   0   0|2292M    0 | 587k    0 |39.3 32.3 28.5|9220M 16.7M  135M  242G|   0     0 | 752k 1298k| 1792  22507
10-12 00:16:55|  3   9  62  25   0   0|2292M    0 | 587k    0 |39.3 32.3 28.5|9239M 16.7M  135M  242G|   0     0 | 729k 1299k| 1792  22511
10-12 00:16:56|  3   9  62  26   0   0|2292M   52k| 587k 4.00 |38.6 32.3 28.5|9242M 16.8M  135M  242G|   0     0 | 725k 1299k| 1792  22511

00:16:57 DEBUG (ts: 1544372217.031345) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    0.00    4.00     0.00     0.05    26.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586754.00    0.00  2292.00     0.00     8.00    26.13    0.04    0.04    0.00   0.00 103.40


00:16:58 DEBUG (ts: 1544372218.031414) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586810.00    0.00  2292.23     0.00     8.00    26.01    0.04    0.04    0.00   0.00 102.30


00:16:59 DEBUG (ts: 1544372219.031507) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00  114.00     0.00     0.51     9.12     0.04    0.39    0.00    0.39   0.01   0.10
nvme0n1           0.00     0.00 586889.00    0.00  2292.54     0.00     8.00    26.33    0.04    0.04    0.00   0.00 102.00


00:17:00 DEBUG (ts: 1544372220.031422) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586613.00    0.00  2291.46     0.00     8.00    26.22    0.04    0.04    0.00   0.00 102.80


00:17:01 DEBUG (ts: 1544372221.031443) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586649.00    0.00  2291.60     0.00     8.00    25.82    0.04    0.04    0.00   0.00 101.90


00:17:01 DEBUG (ts: 1544372221.124693) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:16:57|  3   9  62  25   0   0|2292M    0 | 587k    0 |38.6 32.3 28.5|9243M 16.8M  135M  242G|   0     0 | 726k 1299k| 1792  22522
10-12 00:16:58|  3   9  62  26   0   0|2293M  520k| 587k  114 |38.6 32.3 28.5|9246M 16.8M  135M  242G|   0     0 | 725k 1299k| 1792  22511
10-12 00:16:59|  3   9  62  26   0   0|2291M    0 | 587k    0 |38.6 32.3 28.5|9249M 16.8M  135M  242G|   0     0 | 725k 1298k| 1792  22511
10-12 00:17:00|  3   9  62  26   0   0|2290M 4096B| 586k 1.00 |38.6 32.3 28.5|9249M 16.8M  135M  242G|   0     0 | 724k 1298k| 1792  22511
10-12 00:17:01|  3   9  62  26   0   0|2293M   60k| 587k 3.00 |38.0 32.3 28.5|9246M 16.8M  135M  242G|   0     0 | 726k 1300k| 1792  22511

00:17:02 DEBUG (ts: 1544372222.031440) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    12.00    1.00    3.00     0.00     0.06    32.00     0.00    0.25    0.00    0.33   0.25   0.10
nvme0n1           0.00     0.00 586799.00    0.00  2292.18     0.00     8.00    26.50    0.04    0.04    0.00   0.00 103.20


00:17:03 DEBUG (ts: 1544372223.031591) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    7.00    2.00     0.05     0.03    18.67     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586631.00    0.00  2291.53     0.00     8.00    26.41    0.04    0.04    0.00   0.00 103.00


00:17:04 DEBUG (ts: 1544372224.031505) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    9.00     0.00     0.04     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586771.00    0.00  2292.07     0.00     8.00    26.18    0.04    0.04    0.00   0.00 101.80


00:17:05 DEBUG (ts: 1544372225.031522) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586550.00    0.00  2291.21     0.00     8.00    26.50    0.04    0.04    0.00   0.00 102.70


00:17:05 INFO  (ts: 1544372225.095738) engine_race.cc:421: Read Stat of tid 62, elapsed time: 56.076 s, ts: 1544372225.095 s
00:17:05 INFO  (ts: 1544372225.153724) engine_race.cc:421: Read Stat of tid 60, elapsed time: 56.134 s, ts: 1544372225.153 s
00:17:06 DEBUG (ts: 1544372226.031644) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586829.00    0.00  2292.30     0.00     8.00    26.30    0.04    0.04    0.00   0.00 103.20


00:17:06 DEBUG (ts: 1544372226.124591) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:02|  3   9  62  26   0   0|2292M   32k| 587k 2.00 |38.0 32.3 28.5|9248M 16.8M  135M  242G|   0     0 | 725k 1299k| 1792  22518
10-12 00:17:03|  3   9  62  26   0   0|2290M   36k| 586k 9.00 |38.0 32.3 28.5|9240M 16.8M  135M  242G|   0     0 | 725k 1298k| 1792  22529
10-12 00:17:04|  3   9  62  26   0   0|2293M    0 | 587k    0 |38.0 32.3 28.5|9240M 16.8M  135M  242G|   0     0 | 725k 1299k| 1792  22518
10-12 00:17:05|  3   9  62  26   0   0|2291M    0 | 586k    0 |38.0 32.3 28.5|9240M 16.8M  135M  242G|   0     0 | 724k 1298k| 1792  22518
10-12 00:17:06|  3   9  62  26   0   0|2293M  100k| 587k 2.00 |37.6 32.3 28.6|9240M 16.8M  135M  242G|   0     0 | 717k 1292k| 1792  22518

00:17:07 DEBUG (ts: 1544372227.031583) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    23.00    0.00    2.00     0.00     0.10   100.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586637.00    0.00  2291.55     0.00     8.00    26.27    0.04    0.04    0.00   0.00 102.30


00:17:08 DEBUG (ts: 1544372228.031667) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    0.00    2.00     0.00     0.04    36.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586812.00    0.00  2292.23     0.00     8.00    25.99    0.04    0.04    0.00   0.00 102.00


00:17:09 DEBUG (ts: 1544372229.031691) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00   10.00     0.00     0.04     8.73     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586799.00    0.00  2292.18     0.00     8.00    26.24    0.04    0.04    0.00   0.00 102.80


00:17:10 DEBUG (ts: 1544372230.031645) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586758.00    0.00  2292.03     0.00     8.00    26.11    0.04    0.04    0.00   0.00 102.80


00:17:11 DEBUG (ts: 1544372231.031578) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586580.00    0.00  2291.32     0.00     8.00    26.22    0.04    0.04    0.00   0.00 102.60


00:17:11 DEBUG (ts: 1544372231.124570) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:07|  3   9  62  26   0   0|2292M   36k| 587k 2.00 |37.6 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 716k 1292k| 1664  22519
10-12 00:17:08|  3   9  62  26   0   0|2292M   44k| 587k 10.0 |37.6 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 716k 1292k| 1664  22519
10-12 00:17:09|  3   9  62  26   0   0|2292M    0 | 587k    0 |37.6 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 717k 1292k| 1664  22530
10-12 00:17:10|  3   9  62  26   0   0|2291M    0 | 587k    0 |37.2 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 719k 1291k| 1664  22519
10-12 00:17:11|  3   9  62  26   0   0|2291M    0 | 587k    0 |37.2 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 718k 1291k| 1664  22519

00:17:12 DEBUG (ts: 1544372232.031678) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586545.00    0.00  2291.20     0.00     8.00    26.16    0.04    0.04    0.00   0.00 102.20


00:17:13 DEBUG (ts: 1544372233.031704) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     4.00    0.00    2.00     0.00     0.02    24.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 586904.00    0.00  2292.59     0.00     8.00    26.04    0.04    0.04    0.00   0.00 101.80


00:17:14 DEBUG (ts: 1544372234.031693) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    11.00    0.00    2.00     0.00     0.05    52.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586920.00    0.00  2292.66     0.00     8.00    26.27    0.04    0.04    0.00   0.00 102.30


00:17:15 DEBUG (ts: 1544372235.031794) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586436.00    0.00  2290.77     0.00     8.00    26.39    0.04    0.04    0.00   0.00 102.50


00:17:16 DEBUG (ts: 1544372236.031797) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586678.00    0.00  2291.70     0.00     8.00    26.47    0.04    0.04    0.00   0.00 102.30


00:17:16 DEBUG (ts: 1544372236.124613) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:12|  3   9  62  26   0   0|2292M   24k| 587k 2.00 |37.2 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 720k 1291k| 1664  22519
10-12 00:17:13|  3   9  63  26   0   0|2293M   52k| 587k 2.00 |37.2 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 721k 1292k| 1664  22519
10-12 00:17:14|  3   9  62  26   0   0|2291M    0 | 587k    0 |36.8 32.3 28.6|9246M 16.8M  135M  242G|   0     0 | 718k 1291k| 1664  22530
10-12 00:17:15|  3   9  62  26   0   0|2290M    0 | 586k    0 |36.8 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 721k 1291k| 1664  22530
10-12 00:17:16|  3   9  62  26   0   0|2294M    0 | 587k    0 |36.8 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 722k 1293k| 1664  22519

00:17:17 DEBUG (ts: 1544372237.031719) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586851.00    0.00  2292.39     0.00     8.00    26.24    0.04    0.04    0.00   0.00 102.20


00:17:18 DEBUG (ts: 1544372238.031880) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586764.00    0.00  2292.05     0.00     8.00    26.23    0.04    0.04    0.00   0.00 102.60


00:17:19 DEBUG (ts: 1544372239.031838) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 586885.00    0.00  2292.52     0.00     8.00    26.38    0.04    0.04    0.00   0.00 102.70


00:17:20 DEBUG (ts: 1544372240.031800) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586600.00    0.00  2291.40     0.00     8.00    25.89    0.04    0.04    0.00   0.00 102.00


00:17:21 DEBUG (ts: 1544372241.031834) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586564.00    0.00  2291.27     0.00     8.00    26.18    0.04    0.04    0.00   0.00 102.00


00:17:21 DEBUG (ts: 1544372241.124593) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:17|  3   9  62  25   0   0|2292M    0 | 587k    0 |36.8 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 724k 1291k| 1664  22530
10-12 00:17:18|  3   9  63  26   0   0|2292M   32k| 587k 2.00 |36.3 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 721k 1291k| 1664  22519
10-12 00:17:19|  3   9  62  26   0   0|2291M    0 | 587k    0 |36.3 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 719k 1291k| 1664  22519
10-12 00:17:20|  3   9  62  26   0   0|2291M   28k| 587k 2.00 |36.3 32.3 28.6|9248M 16.8M  135M  242G|   0     0 | 718k 1291k| 1664  22519
10-12 00:17:21|  3   9  62  26   0   0|2292M    0 | 587k    0 |36.3 32.3 28.6|9249M 16.8M  135M  242G|   0     0 | 722k 1292k| 1536  22530

00:17:22 DEBUG (ts: 1544372242.031845) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586761.00    0.00  2292.04     0.00     8.00    26.11    0.04    0.04    0.00   0.00 102.20


00:17:23 DEBUG (ts: 1544372243.031931) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586468.00    0.00  2290.89     0.00     8.00    26.25    0.04    0.04    0.00   0.00 101.80


00:17:24 DEBUG (ts: 1544372244.031873) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00   14.00     0.00     0.09    13.14     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586697.00    0.00  2291.79     0.00     8.00    26.36    0.04    0.04    0.00   0.00 102.90


00:17:25 DEBUG (ts: 1544372245.031882) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 586586.00    0.00  2291.35     0.00     8.00    26.36    0.04    0.04    0.00   0.00 103.00


00:17:26 DEBUG (ts: 1544372246.032006) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586585.00    0.00  2291.35     0.00     8.00    25.99    0.04    0.04    0.00   0.00 102.00


00:17:26 DEBUG (ts: 1544372246.124514) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:22|  3   9  62  26   0   0|2291M    0 | 586k    0 |36.0 32.3 28.6|9249M 16.8M  135M  242G|   0     0 | 719k 1291k| 1536  22519
10-12 00:17:23|  3   9  62  26   0   0|2291M   92k| 587k 14.0 |36.0 32.3 28.6|9249M 16.8M  135M  242G|   0     0 | 719k 1291k| 1536  22519
10-12 00:17:24|  3   9  63  25   0   0|2290M   28k| 586k 2.00 |36.0 32.3 28.6|9249M 16.8M  135M  242G|   0     0 | 721k 1290k| 1536  22519
10-12 00:17:25|  3   9  62  26   0   0|2293M    0 | 587k    0 |36.0 32.3 28.6|9249M 16.8M  135M  242G|   0     0 | 722k 1292k| 1536  22519
10-12 00:17:26|  3   9  62  26   0   0|2290M   28k| 586k 2.00 |36.0 32.3 28.6|9249M 16.8M  135M  242G|   0     0 | 721k 1290k| 1536  22519

00:17:27 DEBUG (ts: 1544372247.032017) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586355.00    0.00  2290.45     0.00     8.00    26.24    0.04    0.04    0.00   0.00 102.10


00:17:28 DEBUG (ts: 1544372248.032030) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586472.00    0.00  2290.91     0.00     8.00    26.00    0.04    0.04    0.00   0.00 101.20


00:17:29 DEBUG (ts: 1544372249.032015) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    4.00     0.00     0.02     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586835.00    0.00  2292.32     0.00     8.00    26.06    0.04    0.04    0.00   0.00 101.70


00:17:30 DEBUG (ts: 1544372250.032056) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586419.00    0.00  2290.70     0.00     8.00    26.26    0.04    0.04    0.00   0.00 102.60


00:17:31 DEBUG (ts: 1544372251.032124) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    12.00    0.00    4.00     0.00     0.06    32.00     0.00    0.25    0.00    0.25   0.25   0.10
nvme0n1           0.00     0.00 586566.00    0.00  2291.28     0.00     8.00    26.25    0.04    0.04    0.00   0.00 102.80


00:17:31 DEBUG (ts: 1544372251.124519) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:27|  3   9  62  26   0   0|2291M    0 | 587k    0 |35.6 32.3 28.6|9247M 16.8M  135M  242G|   0     0 | 719k 1291k| 1664  22530
10-12 00:17:28|  3   9  62  26   0   0|2292M   16k| 587k 4.00 |35.6 32.3 28.6|9247M 16.8M  135M  242G|   0     0 | 718k 1292k| 1664  22519
10-12 00:17:29|  3   9  62  26   0   0|2290M    0 | 586k    0 |35.6 32.3 28.6|9245M 16.8M  135M  242G|   0     0 | 717k 1291k| 1664  22520
10-12 00:17:30|  3   9  62  26   0   0|2291M   64k| 587k 4.00 |35.6 32.3 28.6|9246M 16.8M  135M  242G|   0     0 | 718k 1292k| 1664  22520
10-12 00:17:31|  3   9  62  26   0   0|2292M 4096B| 587k 1.00 |35.3 32.2 28.7|9244M 16.8M  135M  242G|   0     0 | 719k 1292k| 1920  22531

00:17:32 DEBUG (ts: 1544372252.032090) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586661.00    0.00  2291.64     0.00     8.00    26.76    0.04    0.04    0.00   0.00 104.30


00:17:33 DEBUG (ts: 1544372253.032047) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586688.00    0.00  2291.75     0.00     8.00    26.55    0.04    0.04    0.00   0.00 102.90


00:17:34 DEBUG (ts: 1544372254.032174) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    2.00  125.00     0.01     0.55     8.94     0.06    0.47    0.50    0.47   0.02   0.20
nvme0n1           0.00     0.00 586858.00    0.00  2292.41     0.00     8.00    26.33    0.04    0.04    0.00   0.00 102.60


00:17:35 DEBUG (ts: 1544372255.032076) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586438.00    0.00  2290.77     0.00     8.00    26.02    0.04    0.04    0.00   0.00 102.00


00:17:36 DEBUG (ts: 1544372256.032108) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    23.00    0.00    2.00     0.00     0.10   100.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586465.00    0.00  2290.88     0.00     8.00    26.35    0.04    0.04    0.00   0.00 102.60


00:17:36 DEBUG (ts: 1544372256.124548) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:32|  3   9  62  26   0   0|2292M   28k| 587k 2.00 |35.3 32.2 28.7|9242M 16.9M  135M  242G|   0     0 | 718k 1292k| 1920  22520
10-12 00:17:33|  3   9  62  26   0   0|2292M  560k| 587k  125 |35.3 32.2 28.7|9243M 16.9M  135M  242G|   0     0 | 719k 1292k| 2048  22531
10-12 00:17:34|  3   9  62  26   0   0|2291M    0 | 586k    0 |35.3 32.2 28.7|9242M 16.9M  135M  242G|   0     0 | 717k 1291k| 1920  22520
10-12 00:17:35|  3   9  62  26   0   0|2291M  100k| 586k 2.00 |35.3 32.2 28.7|9242M 16.9M  135M  242G|   0     0 | 727k 1291k| 1920  22520
10-12 00:17:36|  3   9  62  26   0   0|2292M    0 | 587k    0 |35.3 32.2 28.7|9242M 16.9M  135M  242G|   0     0 | 735k 1291k| 1920  22520

00:17:37 DEBUG (ts: 1544372257.032210) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586615.00    0.00  2291.46     0.00     8.00    26.14    0.04    0.04    0.00   0.00 102.60


00:17:38 DEBUG (ts: 1544372258.032155) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586793.00    0.00  2292.16     0.00     8.00    26.37    0.04    0.04    0.00   0.00 103.20


00:17:39 DEBUG (ts: 1544372259.032251) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    12.00    0.00   18.00     0.00     0.12    13.78     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586827.00    0.00  2292.29     0.00     8.00    26.64    0.04    0.04    0.00   0.00 103.50


00:17:40 DEBUG (ts: 1544372260.032199) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586625.00    0.00  2291.50     0.00     8.00    26.12    0.04    0.04    0.00   0.00 101.90


00:17:41 DEBUG (ts: 1544372261.032172) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    11.00    0.00    2.00     0.00     0.05    52.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 586642.00    0.00  2291.57     0.00     8.00    26.74    0.04    0.04    0.00   0.00 103.10


00:17:41 DEBUG (ts: 1544372261.124005) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:37|  3   9  62  26   0   0|2292M    0 | 587k    0 |35.3 32.2 28.7|9242M 16.9M  135M  242G|   0     0 | 735k 1291k| 1920  22520
10-12 00:17:38|  3   9  62  26   0   0|2292M  124k| 587k 18.0 |35.2 32.3 28.7|9242M 16.9M  135M  242G|   0     0 | 717k 1291k| 1920  22520
10-12 00:17:39|  3   9  62  26   0   0|2291M    0 | 587k    0 |35.2 32.3 28.7|9247M 16.9M  135M  242G|   0     0 | 718k 1292k| 1920  22532
10-12 00:17:40|  3   9  62  26   0   0|2292M   52k| 587k 2.00 |35.2 32.3 28.7|9246M 16.9M  135M  242G|   0     0 | 717k 1292k| 1920  22521
10-12 00:17:41|  3   9  62  26   0   0|2290M    0 | 586k    0 |35.2 32.3 28.7|9246M 16.9M  135M  242G|   0     0 | 719k 1291k| 1920  22521

00:17:42 DEBUG (ts: 1544372262.032311) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586625.00    0.00  2291.50     0.00     8.00    26.42    0.04    0.04    0.00   0.00 102.80


00:17:43 DEBUG (ts: 1544372263.032255) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 587007.00    0.00  2292.99     0.00     8.00    26.41    0.04    0.04    0.00   0.00 102.80


00:17:44 DEBUG (ts: 1544372264.032278) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    5.00     0.00     0.03    11.20     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586629.00    0.00  2291.52     0.00     8.00    26.22    0.04    0.04    0.00   0.00 102.40


00:17:45 DEBUG (ts: 1544372265.032249) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    0.00    2.00     0.00     0.04    44.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586527.00    0.00  2291.12     0.00     8.00    26.21    0.04    0.04    0.00   0.00 102.10


00:17:46 DEBUG (ts: 1544372266.032238) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     3.00    0.00    2.00     0.00     0.02    20.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586954.00    0.00  2292.79     0.00     8.00    26.61    0.04    0.04    0.00   0.00 103.30


00:17:46 DEBUG (ts: 1544372266.124558) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:42|  3   9  62  26   0   0|2293M    0 | 587k    0 |35.2 32.3 28.7|9246M 16.9M  135M  242G|   0     0 | 716k 1292k| 1920  22521
10-12 00:17:43|  3   9  62  26   0   0|2293M   28k| 587k 5.00 |35.2 32.3 28.7|9246M 16.9M  135M  242G|   0     0 | 718k 1292k| 1920  22521
10-12 00:17:44|  3   9  62  26   0   0|2290M   44k| 586k 2.00 |34.9 32.3 28.7|9246M 16.9M  135M  242G|   0     0 | 715k 1290k| 1920  22521
10-12 00:17:45|  3   9  62  26   0   0|2293M   20k| 587k 2.00 |34.9 32.3 28.7|9244M 16.9M  135M  242G|   0     0 | 718k 1293k| 1920  22534
10-12 00:17:46|  3   9  62  26   0   0|2292M    0 | 587k    0 |34.9 32.3 28.7|9243M 16.9M  135M  242G|   0     0 | 716k 1291k| 1920  22521

00:17:47 DEBUG (ts: 1544372267.032284) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586279.00    0.00  2290.15     0.00     8.00    26.25    0.04    0.04    0.00   0.00 102.70


00:17:47 DEBUG (ts: 1544372267.123927) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:47|  3   9  62  26   0   0|2289M    0 | 586k    0 |34.9 32.3 28.7|9243M 16.9M  135M  242G|   0     0 | 716k 1290k| 1920  22521

00:17:48 DEBUG (ts: 1544372268.032261) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586430.00    0.00  2290.74     0.00     8.00    26.29    0.04    0.04    0.00   0.00 102.70


00:17:48 DEBUG (ts: 1544372268.124557) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:48|  3   9  63  26   0   0|2294M   32k| 587k 5.00 |34.9 32.3 28.7|9243M 16.9M  135M  242G|   0     0 | 721k 1292k| 1920  22521

00:17:49 DEBUG (ts: 1544372269.032340) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    2.00    5.00     0.01     0.03    11.43     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586771.00    0.00  2292.07     0.00     8.00    26.05    0.04    0.04    0.00   0.00 101.40


00:17:49 DEBUG (ts: 1544372269.124546) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:49|  3   9  62  26   0   0|2292M    0 | 587k    0 |34.9 32.3 28.7|9243M 16.9M  135M  242G|   0     0 | 718k 1292k| 1920  22532

00:17:50 DEBUG (ts: 1544372270.032383) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586856.00    0.00  2292.41     0.00     8.00    26.54    0.04    0.04    0.00   0.00 102.50


00:17:50 DEBUG (ts: 1544372270.124549) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:50|  3   9  62  26   0   0|2292M   32k| 587k 2.00 |34.7 32.3 28.7|9254M 16.9M  135M  242G|   0     0 | 716k 1292k| 2048  22521

00:17:51 DEBUG (ts: 1544372271.032429) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586738.00    0.00  2291.95     0.00     8.00    26.29    0.04    0.04    0.00   0.00 102.30


00:17:51 DEBUG (ts: 1544372271.124560) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:51|  3   9  62  26   0   0|2291M   56k| 587k 2.00 |34.7 32.3 28.7|9250M 16.9M  135M  242G|   0     0 | 716k 1292k| 2048  22532

00:17:52 DEBUG (ts: 1544372272.032455) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    12.00    0.00    2.00     0.00     0.05    56.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586594.00    0.00  2291.38     0.00     8.00    26.28    0.04    0.04    0.00   0.00 102.50


00:17:52 DEBUG (ts: 1544372272.124542) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:52|  3   9  62  26   0   0|2291M    0 | 586k    0 |34.7 32.3 28.7|9250M 16.9M  135M  242G|   0     0 | 715k 1291k| 2048  22521

00:17:53 DEBUG (ts: 1544372273.032392) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586478.00    0.00  2290.93     0.00     8.00    26.36    0.04    0.04    0.00   0.00 102.80


00:17:53 DEBUG (ts: 1544372273.123939) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:53|  3   9  62  26   0   0|2289M    0 | 586k    0 |34.7 32.3 28.7|9250M 16.9M  135M  242G|   0     0 | 714k 1290k| 2048  22515

00:17:54 DEBUG (ts: 1544372274.124565) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 00:17:54|  3   9  62  26   0   0|2293M    0 | 587k    0 |34.7 32.3 28.7|9250M 16.9M  135M  242G|   0     0 | 715k 1292k| 2048  22515

00:17:54 INFO  (ts: 1544372274.635860) engine_race.cc:421: Read Stat of tid 23, elapsed time: 105.616 s, ts: 1544372274.635 s
00:17:54 INFO  (ts: 1544372274.676626) engine_race.cc:421: Read Stat of tid 41, elapsed time: 105.657 s, ts: 1544372274.676 s
00:17:54 INFO  (ts: 1544372274.754970) engine_race.cc:421: Read Stat of tid 61, elapsed time: 105.735 s, ts: 1544372274.754 s
00:17:54 INFO  (ts: 1544372274.755917) engine_race.cc:421: Read Stat of tid 2, elapsed time: 105.736 s, ts: 1544372274.755 s
00:17:54 INFO  (ts: 1544372274.756593) engine_race.cc:421: Read Stat of tid 34, elapsed time: 105.737 s, ts: 1544372274.756 s
00:17:54 INFO  (ts: 1544372274.760306) engine_race.cc:421: Read Stat of tid 46, elapsed time: 105.740 s, ts: 1544372274.760 s
00:17:54 INFO  (ts: 1544372274.778420) engine_race.cc:421: Read Stat of tid 43, elapsed time: 105.758 s, ts: 1544372274.778 s
00:17:54 INFO  (ts: 1544372274.779133) engine_race.cc:421: Read Stat of tid 57, elapsed time: 105.759 s, ts: 1544372274.779 s
00:17:54 INFO  (ts: 1544372274.781342) engine_race.cc:421: Read Stat of tid 1, elapsed time: 105.761 s, ts: 1544372274.781 s
00:17:54 INFO  (ts: 1544372274.782401) engine_race.cc:421: Read Stat of tid 29, elapsed time: 105.762 s, ts: 1544372274.782 s
00:17:54 INFO  (ts: 1544372274.793507) engine_race.cc:421: Read Stat of tid 45, elapsed time: 105.773 s, ts: 1544372274.793 s
00:17:54 INFO  (ts: 1544372274.798429) engine_race.cc:421: Read Stat of tid 40, elapsed time: 105.778 s, ts: 1544372274.798 s
00:17:54 INFO  (ts: 1544372274.800040) engine_race.cc:421: Read Stat of tid 38, elapsed time: 105.780 s, ts: 1544372274.800 s
00:17:54 INFO  (ts: 1544372274.814265) engine_race.cc:421: Read Stat of tid 36, elapsed time: 105.794 s, ts: 1544372274.814 s
00:17:54 INFO  (ts: 1544372274.823764) engine_race.cc:421: Read Stat of tid 58, elapsed time: 105.804 s, ts: 1544372274.823 s
00:17:54 INFO  (ts: 1544372274.825279) engine_race.cc:421: Read Stat of tid 10, elapsed time: 105.805 s, ts: 1544372274.825 s
00:17:54 INFO  (ts: 1544372274.828703) engine_race.cc:421: Read Stat of tid 14, elapsed time: 105.809 s, ts: 1544372274.828 s
00:17:54 INFO  (ts: 1544372274.831274) engine_race.cc:421: Read Stat of tid 25, elapsed time: 105.811 s, ts: 1544372274.831 s
00:17:54 INFO  (ts: 1544372274.833708) engine_race.cc:421: Read Stat of tid 28, elapsed time: 105.814 s, ts: 1544372274.833 s
00:17:54 INFO  (ts: 1544372274.834130) engine_race.cc:421: Read Stat of tid 9, elapsed time: 105.814 s, ts: 1544372274.834 s
00:17:54 INFO  (ts: 1544372274.836006) engine_race.cc:421: Read Stat of tid 32, elapsed time: 105.816 s, ts: 1544372274.836 s
00:17:54 INFO  (ts: 1544372274.839902) engine_race.cc:421: Read Stat of tid 20, elapsed time: 105.820 s, ts: 1544372274.839 s
00:17:54 INFO  (ts: 1544372274.850639) engine_race.cc:421: Read Stat of tid 4, elapsed time: 105.831 s, ts: 1544372274.850 s
00:17:54 INFO  (ts: 1544372274.873221) engine_race.cc:421: Read Stat of tid 6, elapsed time: 105.853 s, ts: 1544372274.873 s
00:17:54 INFO  (ts: 1544372274.874985) engine_race.cc:421: Read Stat of tid 27, elapsed time: 105.855 s, ts: 1544372274.874 s
00:17:54 INFO  (ts: 1544372274.877594) engine_race.cc:421: Read Stat of tid 49, elapsed time: 105.858 s, ts: 1544372274.877 s
00:17:54 INFO  (ts: 1544372274.882041) engine_race.cc:421: Read Stat of tid 47, elapsed time: 105.862 s, ts: 1544372274.882 s
00:17:54 INFO  (ts: 1544372274.882108) engine_race.cc:421: Read Stat of tid 16, elapsed time: 105.862 s, ts: 1544372274.882 s
00:17:54 INFO  (ts: 1544372274.883647) engine_race.cc:421: Read Stat of tid 3, elapsed time: 105.864 s, ts: 1544372274.883 s
00:17:54 INFO  (ts: 1544372274.886197) engine_race.cc:421: Read Stat of tid 8, elapsed time: 105.866 s, ts: 1544372274.886 s
00:17:54 INFO  (ts: 1544372274.888262) engine_race.cc:421: Read Stat of tid 12, elapsed time: 105.868 s, ts: 1544372274.888 s
00:17:54 INFO  (ts: 1544372274.888902) engine_race.cc:421: Read Stat of tid 63, elapsed time: 105.869 s, ts: 1544372274.888 s
00:17:54 INFO  (ts: 1544372274.890590) engine_race.cc:421: Read Stat of tid 37, elapsed time: 105.871 s, ts: 1544372274.890 s
00:17:54 INFO  (ts: 1544372274.893076) engine_race.cc:421: Read Stat of tid 54, elapsed time: 105.873 s, ts: 1544372274.893 s
00:17:54 INFO  (ts: 1544372274.895411) engine_race.cc:421: Read Stat of tid 17, elapsed time: 105.875 s, ts: 1544372274.895 s
00:17:54 INFO  (ts: 1544372274.895786) engine_race.cc:421: Read Stat of tid 21, elapsed time: 105.876 s, ts: 1544372274.895 s
00:17:54 INFO  (ts: 1544372274.897865) engine_race.cc:421: Read Stat of tid 59, elapsed time: 105.878 s, ts: 1544372274.897 s
00:17:54 INFO  (ts: 1544372274.898992) engine_race.cc:421: Read Stat of tid 53, elapsed time: 105.879 s, ts: 1544372274.898 s
00:17:54 INFO  (ts: 1544372274.899268) engine_race.cc:421: Read Stat of tid 7, elapsed time: 105.879 s, ts: 1544372274.899 s
00:17:54 INFO  (ts: 1544372274.899570) engine_race.cc:421: Read Stat of tid 24, elapsed time: 105.880 s, ts: 1544372274.899 s
00:17:54 INFO  (ts: 1544372274.900945) engine_race.cc:421: Read Stat of tid 55, elapsed time: 105.881 s, ts: 1544372274.900 s
00:17:54 INFO  (ts: 1544372274.902825) engine_race.cc:421: Read Stat of tid 48, elapsed time: 105.883 s, ts: 1544372274.902 s
00:17:54 INFO  (ts: 1544372274.903393) engine_race.cc:421: Read Stat of tid 13, elapsed time: 105.883 s, ts: 1544372274.903 s
00:17:54 INFO  (ts: 1544372274.903507) engine_race.cc:421: Read Stat of tid 22, elapsed time: 105.883 s, ts: 1544372274.903 s
00:17:54 INFO  (ts: 1544372274.904047) engine_race.cc:421: Read Stat of tid 33, elapsed time: 105.884 s, ts: 1544372274.904 s
00:17:54 INFO  (ts: 1544372274.905808) engine_race.cc:421: Read Stat of tid 15, elapsed time: 105.886 s, ts: 1544372274.905 s
00:17:54 INFO  (ts: 1544372274.907267) engine_race.cc:421: Read Stat of tid 30, elapsed time: 105.887 s, ts: 1544372274.907 s
00:17:54 INFO  (ts: 1544372274.907867) engine_race.cc:421: Read Stat of tid 42, elapsed time: 105.888 s, ts: 1544372274.907 s
00:17:54 INFO  (ts: 1544372274.908287) engine_race.cc:421: Read Stat of tid 35, elapsed time: 105.888 s, ts: 1544372274.908 s
00:17:54 INFO  (ts: 1544372274.908856) engine_race.cc:421: Read Stat of tid 18, elapsed time: 105.889 s, ts: 1544372274.908 s
00:17:54 INFO  (ts: 1544372274.909026) engine_race.cc:421: Read Stat of tid 50, elapsed time: 105.889 s, ts: 1544372274.909 s
00:17:54 INFO  (ts: 1544372274.909852) engine_race.cc:421: Read Stat of tid 5, elapsed time: 105.890 s, ts: 1544372274.909 s
00:17:54 INFO  (ts: 1544372274.918276) engine_race.cc:421: Read Stat of tid 0, elapsed time: 105.898 s, ts: 1544372274.918 s
00:17:54 INFO  (ts: 1544372274.924346) engine_race.cc:421: Read Stat of tid 26, elapsed time: 105.904 s, ts: 1544372274.924 s
00:17:54 INFO  (ts: 1544372274.925902) engine_race.cc:421: Read Stat of tid 11, elapsed time: 105.906 s, ts: 1544372274.925 s
00:17:54 INFO  (ts: 1544372274.932500) engine_race.cc:421: Read Stat of tid 39, elapsed time: 105.912 s, ts: 1544372274.932 s
00:17:54 INFO  (ts: 1544372274.933831) engine_race.cc:421: Read Stat of tid 52, elapsed time: 105.914 s, ts: 1544372274.933 s
00:17:54 INFO  (ts: 1544372274.937134) engine_race.cc:421: Read Stat of tid 56, elapsed time: 105.917 s, ts: 1544372274.937 s
00:17:54 INFO  (ts: 1544372274.942745) engine_race.cc:421: Read Stat of tid 19, elapsed time: 105.923 s, ts: 1544372274.942 s
00:17:54 INFO  (ts: 1544372274.943724) engine_race.cc:421: Read Stat of tid 31, elapsed time: 105.924 s, ts: 1544372274.943 s
00:17:54 INFO  (ts: 1544372274.944546) engine_race.cc:421: Read Stat of tid 44, elapsed time: 105.924 s, ts: 1544372274.944 s
00:17:54 INFO  (ts: 1544372274.944711) engine_race.cc:421: Read Stat of tid 51, elapsed time: 105.925 s, ts: 1544372274.944 s
readrandom   :       1.652 micros/op 605439 ops/sec; 2295.6 MB/s (1000000 of 1000000 found)

Microseconds per read:
Count: 64000000 Average: 104.0732  StdDev: 40.43
Min: 0  Median: 57.5245  Max: 4362
Percentiles: P50: 57.52 P75: 69.23 P99: 851.72 P99.9: 1260.42 P99.99: 1801.53
------------------------------------------------------
[       0,       1 ]  1494469   2.335%   2.335%
(       1,       2 ]   103712   0.162%   2.497%
(       2,       3 ]   122252   0.191%   2.688%
(       3,       4 ]    51325   0.080%   2.768%
(       4,       6 ]    21741   0.034%   2.802%
(       6,      10 ]     6913   0.011%   2.813%
(      10,      15 ]    13956   0.022%   2.835%
(      15,      22 ]    11498   0.018%   2.853%
(      22,      34 ]    13864   0.022%   2.875%
(      34,      51 ] 21242970  33.192%  36.067% #######
(      51,      76 ] 34168521  53.388%  89.455% ###########
(      76,     110 ]   819205   1.280%  90.735%
(     110,     170 ]    14922   0.023%  90.758%
(     170,     250 ]    18955   0.030%  90.788%
(     250,     380 ]   131885   0.206%  90.994%
(     380,     580 ]  1645250   2.571%  93.565% #
(     580,     870 ]  3712568   5.801%  99.366% #
(     870,    1300 ]   376664   0.589%  99.954%
(    1300,    1900 ]    27432   0.043%  99.997%
(    1900,    2900 ]     1872   0.003% 100.000%
(    2900,    4400 ]       26   0.000% 100.000%


------------------------------------------------
!!!Competion Report!!!
         readrandom: 605439 ops/second
disk usage: 278172 MB
------------------------------------------------
00:17:54 (Func: ~EngineRace, Line: 247), (TS: 1544372274.945148 s)-(Elapsed: 105.925596 s), (Mem: 1105.195312 MB)

00:17:54 (Func: ~EngineRace, Line: 310), (TS: 1544372274.945274 s)-(Elapsed: 105.925723 s), (Mem: 1105.417969 MB)

00:17:54 (Func: ~EngineRace, Line: 344), (TS: 1544372274.945363 s)-(Elapsed: 105.925812 s), (Mem: 1105.414062 MB)
```

* shrink 2

```
------------------------------------------------
readrandom
01:01:08 INFO  (ts: 1544374868.805856) engine_race.cc:89: sizeof 8, 8, 10
01:01:08 (Func: Open, Line: 228), (TS: 1544374868.805934 s)-(Elapsed: 0.000079 s), (Mem: 490.410156 MB)

01:01:08 (Func: EngineRace, Line: 111), (TS: 1544374868.806074 s)-(Elapsed: 0.000218 s), (Mem: 490.449219 MB)

01:01:08 (Func: EngineRace, Line: 191), (TS: 1544374868.813385 s)-(Elapsed: 0.007529 s), (Mem: 490.468750 MB)

01:01:08 (Func: EngineRace, Line: 214), (TS: 1544374868.813672 s)-(Elapsed: 0.007817 s), (Mem: 490.472656 MB)

01:01:08 INFO  (ts: 1544374868.813709) engine_race.cc:907: After Flush Files, time: 0.007 s
01:01:08 (Func: EngineRace, Line: 216), (TS: 1544374868.813722 s)-(Elapsed: 0.007866 s), (Mem: 490.722656 MB)

01:01:08 DEBUG (ts: 1544374868.816983) util.h:109:


01:01:09 (Func: EngineRace, Line: 220), (TS: 1544374869.019453 s)-(Elapsed: 0.213597 s), (Mem: 1103.238281 MB)

01:01:09 (Func: EngineRace, Line: 223), (TS: 1544374869.019538 s)-(Elapsed: 0.213682 s), (Mem: 1103.238281 MB)

01:01:09 (Func: Open, Line: 242), (TS: 1544374869.019571 s)-(Elapsed: 0.213715 s), (Mem: 1103.238281 MB)

DB path: [test_directory]
01:01:09 INFO  (ts: 1544374869.021801) engine_race.cc:475: not found in tid: 62

01:01:09 INFO  (ts: 1544374869.021931) engine_race.cc:475: not found in tid: 59

01:01:09 DEBUG (ts: 1544374869.816746) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.04     3.20    4.39    8.80     0.12     0.06    27.94     0.01    0.48    0.67    0.38   0.08   0.11
nvme0n1           0.00   656.45 26523.52 4481.96   164.47   111.47    18.23     0.65    0.02    0.01    0.07   0.01  15.62


01:01:10 DEBUG (ts: 1544374870.816832) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda              31.00     0.00 1184.00    0.00    15.71     0.00    27.18     0.90    0.76    0.76    0.00   0.14  16.80
nvme0n1           0.00     0.00 474629.00    0.00  2312.09     0.00     9.98    24.07    0.05    0.05    0.00   0.00 100.10


01:01:10 DEBUG (ts: 1544374870.918546) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
[7l----system---- ----total-cpu-usage---- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:08|  5   4  89   2   0   0| 165M  112M|26.5k 4491 |49.0 29.6 29.0|8307M 13.5M  117M  243G|   0     0 | 124k  113k| 2176  18401
10-12 01:01:09| 14  11  57  19   0   0|2302M    0 | 530k    0 |49.0 29.6 29.0|8645M 15.8M  127M  243G|   0     0 | 999k 1605k| 2048  20384
10-12 01:01:10|  9  12  60  19   0   0|2293M   92k| 587k 2.00 |47.7 29.6 29.0|8667M 16.4M  128M  243G|   0     0 |1105k 1781k| 1920  20495

01:01:11 DEBUG (ts: 1544374871.816872) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    21.00  172.00    2.00     1.21     0.09    15.31     0.12    0.70    0.71    0.00   0.08   1.40
nvme0n1           0.00     0.00 586895.00    0.00  2292.55     0.00     8.00    23.80    0.04    0.04    0.00   0.00 101.30


01:01:12 DEBUG (ts: 1544374872.816899) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   39.00    6.00     0.41     0.03    19.73     0.01    0.27    0.31    0.00   0.09   0.40
nvme0n1           0.00     0.00 586813.00    0.00  2292.24     0.00     8.00    24.12    0.04    0.04    0.00   0.00 102.40


01:01:13 DEBUG (ts: 1544374873.816877) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   34.00    0.00     0.13     0.00     8.00     0.02    0.62    0.62    0.00   0.06   0.20
nvme0n1           0.00     0.00 586859.00    0.00  2292.42     0.00     8.00    23.74    0.04    0.04    0.00   0.00 101.60


01:01:14 DEBUG (ts: 1544374874.816922) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    11.00   62.00    2.00     0.65     0.05    22.38     0.03    0.48    0.50    0.00   0.12   0.80
nvme0n1           0.00   368.00 586245.00   29.00  2290.02     1.55     8.01    23.91    0.04    0.04    0.10   0.00 102.20


01:01:15 DEBUG (ts: 1544374875.816969) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586974.00    0.00  2292.87     0.00     8.00    23.87    0.04    0.04    0.00   0.00 101.90


01:01:15 DEBUG (ts: 1544374875.918589) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:11|  8  11  61  19   0   0|2293M   28k| 587k 6.00 |47.7 29.6 29.0|8693M 16.5M  129M  243G|   0     0 |1096k 1776k| 1792  22093
10-12 01:01:12|  8  11  61  19   0   0|2293M    0 | 587k    0 |47.7 29.6 29.0|8712M 16.5M  129M  243G|   0     0 |1092k 1778k| 1792  22082
10-12 01:01:13|  9  12  61  19   0   0|2291M 1656k| 586k 33.0 |46.3 29.6 29.0|8731M 16.7M  129M  243G|   0     0 |1091k 1776k| 1792  22117
10-12 01:01:14|  9  11  60  19   0   0|2292M    0 | 587k    0 |46.3 29.6 29.0|8751M 16.7M  130M  243G|   0     0 |1104k 1779k| 1792  22106
10-12 01:01:15|  9  12  60  19   0   0|2300M    0 | 587k    0 |46.3 29.6 29.0|8769M 17.6M  137M  243G|   0     0 |1134k 1777k| 1664  23757

01:01:16 DEBUG (ts: 1544374876.816929) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00  344.00    0.00     8.29     0.00    49.37     0.32    0.93    0.93    0.00   0.20   6.80
nvme0n1           0.00     0.00 586394.00    0.00  2290.59     0.00     8.00    23.68    0.04    0.04    0.00   0.00 102.10


01:01:17 DEBUG (ts: 1544374877.816782) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    11.00    5.00  229.00     0.02     1.17    10.46     0.47    2.02    1.80    2.02   0.04   1.00
nvme0n1           0.00     0.00 586552.00    0.00  2291.23     0.00     8.00    23.82    0.04    0.04    0.00   0.00 101.70


01:01:18 DEBUG (ts: 1544374878.816980) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586527.00    0.00  2291.12     0.00     8.00    23.64    0.04    0.04    0.00   0.00 101.20


01:01:19 DEBUG (ts: 1544374879.817006) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    0.00    2.00     0.00     0.04    44.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00   211.00 586360.00   87.00  2290.46     1.16     8.00    23.82    0.04    0.04    0.26   0.00 102.70


01:01:20 DEBUG (ts: 1544374880.817037) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586787.00    0.00  2292.14     0.00     8.00    23.71    0.04    0.04    0.00   0.00 101.70


01:01:20 DEBUG (ts: 1544374880.918508) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:16|  9  12  61  19   0   0|2291M 1200k| 587k  229 |46.3 29.6 29.0|8787M 17.6M  137M  243G|   0     0 |1137k 1775k| 1664  23769
10-12 01:01:17|  9  12  61  19   0   0|2291M    0 | 587k    0 |45.3 29.7 29.1|8806M 17.6M  137M  243G|   0     0 |1098k 1774k| 1792  23769
10-12 01:01:18|  8  12  61  19   0   0|2290M 1236k| 586k 89.0 |45.3 29.7 29.1|8825M 17.6M  137M  243G|   0     0 |1094k 1774k| 1792  23758
10-12 01:01:19|  8  12  61  19   0   0|2292M    0 | 587k    0 |45.3 29.7 29.1|8843M 17.6M  137M  243G|   0     0 |1098k 1775k| 1792  23758
10-12 01:01:20|  8  12  61  19   0   0|2294M    0 | 587k    0 |45.3 29.7 29.1|8857M 17.7M  139M  243G|   0     0 |1097k 1776k| 1792  23759

01:01:21 DEBUG (ts: 1544374881.817068) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   55.00    0.00     1.89     0.00    70.40     0.01    0.25    0.25    0.00   0.24   1.30
nvme0n1           0.00     0.00 586858.00    0.00  2292.41     0.00     8.00    23.70    0.04    0.04    0.00   0.00 101.80


01:01:22 DEBUG (ts: 1544374882.817062) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00   21.00     0.00     0.12    12.19     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586970.00    0.00  2292.85     0.00     8.00    23.46    0.04    0.04    0.00   0.00 101.40


01:01:23 DEBUG (ts: 1544374883.816784) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     9.00    1.00    2.00     0.00     0.04    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586773.00    0.00  2292.08     0.00     8.00    23.68    0.04    0.04    0.00   0.00 102.30


01:01:24 DEBUG (ts: 1544374884.816975) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    3.00    2.00     0.01     0.03    17.60     0.00    0.20    0.33    0.00   0.20   0.10
nvme0n1           0.00     0.00 586789.00    0.00  2292.14     0.00     8.00    23.64    0.04    0.04    0.00   0.00 101.80


01:01:25 DEBUG (ts: 1544374885.816980) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    7.00    0.00     0.04     0.00    11.43     0.00    0.57    0.57    0.00   0.14   0.10
nvme0n1           0.00     0.00 586739.00    0.00  2291.95     0.00     8.00    24.01    0.04    0.04    0.00   0.00 103.00


01:01:25 DEBUG (ts: 1544374885.918542) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:21|  8  12  61  19   0   0|2293M  128k| 587k 21.0 |45.3 29.7 29.1|8874M 17.7M  139M  243G|   0     0 |1100k 1775k| 1792  23757
10-12 01:01:22|  8  12  61  19   0   0|2292M   44k| 587k 2.00 |44.4 29.8 29.1|8901M 17.7M  139M  243G|   0     0 |1099k 1775k| 1792  23758
10-12 01:01:23|  8  12  61  19   0   0|2293M   32k| 587k 2.00 |44.4 29.8 29.1|8914M 17.7M  139M  243G|   0     0 |1104k 1775k| 1792  23769
10-12 01:01:24|  8  12  61  19   0   0|2291M    0 | 587k    0 |44.4 29.8 29.1|8930M 17.7M  139M  243G|   0     0 |1098k 1774k| 1792  23754
10-12 01:01:25|  8  12  61  19   0   0|2292M    0 | 587k    0 |44.4 29.8 29.1|8945M 17.7M  139M  243G|   0     0 |1100k 1775k| 1792  23754

01:01:26 DEBUG (ts: 1544374886.817017) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586783.00    0.00  2292.12     0.00     8.00    23.75    0.04    0.04    0.00   0.00 102.20


01:01:27 DEBUG (ts: 1544374887.817028) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     3.00    3.00   24.00     0.01     0.11     8.89     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586734.00    0.00  2291.93     0.00     8.00    23.73    0.04    0.04    0.00   0.00 102.60


01:01:28 DEBUG (ts: 1544374888.817055) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586742.00    0.00  2291.96     0.00     8.00    23.72    0.04    0.04    0.00   0.00 102.10


01:01:29 DEBUG (ts: 1544374889.817161) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    18.00   36.00   68.00     2.94     0.35    64.85     0.02    0.17    0.50    0.00   0.15   1.60
nvme0n1           0.00     0.00 586641.00    0.00  2291.57     0.00     8.00    23.64    0.04    0.04    0.00   0.00 102.10


01:01:30 DEBUG (ts: 1544374890.817050) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586799.00    0.00  2292.18     0.00     8.00    23.90    0.04    0.04    0.00   0.00 102.20


01:01:30 DEBUG (ts: 1544374890.918118) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:26|  9  12  61  19   0   0|2292M  108k| 587k 24.0 |43.5 29.8 29.1|8960M 17.7M  139M  243G|   0     0 |1098k 1775k| 1792  23757
10-12 01:01:27|  9  12  60  19   0   0|2291M    0 | 587k    0 |43.5 29.8 29.1|8976M 17.7M  139M  243G|   0     0 |1095k 1776k| 1792  23757
10-12 01:01:28|  8  12  61  19   0   0|2295M  360k| 587k 68.0 |43.5 29.8 29.1|8985M 17.7M  142M  243G|   0     0 |1099k 1777k| 1664  23761
10-12 01:01:29|  8  12  61  19   0   0|2291M   16k| 587k 3.00 |43.5 29.8 29.1|8999M 17.7M  142M  243G|   0     0 |1099k 1775k| 1536  23772
10-12 01:01:30|  9  12  61  19   0   0|2293M 4096B| 587k 1.00 |43.5 29.8 29.1|9040M 17.7M  142M  243G|   0     0 |1103k 1775k| 1792  23770

01:01:31 DEBUG (ts: 1544374891.817124) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     1.00    4.00    4.00     0.04     0.02    15.00     0.00    0.12    0.25    0.00   0.12   0.10
nvme0n1           0.00     0.00 586576.00    0.00  2291.31     0.00     8.00    23.59    0.04    0.04    0.00   0.00 102.30


01:01:32 DEBUG (ts: 1544374892.817139) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   49.00    1.00     0.49     0.00    20.32     0.04    0.70    0.71    0.00   0.10   0.50
nvme0n1           0.00     0.00 586839.00    0.00  2292.34     0.00     8.00    23.72    0.04    0.04    0.00   0.00 101.70


01:01:33 DEBUG (ts: 1544374893.817137) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    5.00    0.00     0.06     0.00    25.60     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586805.00    0.00  2292.20     0.00     8.00    23.82    0.04    0.04    0.00   0.00 102.00


01:01:34 DEBUG (ts: 1544374894.817154) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.01     0.00    16.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586914.00    0.00  2292.64     0.00     8.00    23.47    0.04    0.04    0.00   0.00 102.10


01:01:35 DEBUG (ts: 1544374895.817178) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586841.00    0.00  2292.35     0.00     8.00    23.59    0.04    0.04    0.00   0.00 102.70


01:01:35 DEBUG (ts: 1544374895.917823) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:31|  8  12  60  19   0   0|2292M 4096B| 587k 1.00 |42.6 29.9 29.1|9048M 17.9M  142M  243G|   0     0 |1101k 1776k| 1920  23774
10-12 01:01:32|  8  12  60  19   0   0|2292M    0 | 587k    0 |42.6 29.9 29.1|9062M 17.9M  142M  243G|   0     0 |1099k 1775k| 1920  23777
10-12 01:01:33|  8  12  61  19   0   0|2292M    0 | 587k    0 |42.6 29.9 29.1|9073M 17.9M  142M  243G|   0     0 |1099k 1776k| 1920  23778
10-12 01:01:34|  8  12  61  19   0   0|2293M  100k| 587k 2.00 |42.6 29.9 29.1|9084M 17.9M  142M  243G|   0     0 |1098k 1776k| 1920  23778
10-12 01:01:35|  9  12  60  19   0   0|2290M   28k| 586k 2.00 |42.6 29.9 29.1|9085M 17.9M  142M  243G|   0     0 |1099k 1774k| 1920  23790

01:01:36 DEBUG (ts: 1544374896.817189) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    28.00    1.00    4.00     0.00     0.12    52.80     0.00    0.40    0.00    0.50   0.40   0.20
nvme0n1           0.00     0.00 586493.00    0.00  2290.98     0.00     8.00    23.47    0.04    0.04    0.00   0.00 102.30


01:01:37 DEBUG (ts: 1544374897.817194) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    4.00    4.00     0.02     0.02     9.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586376.00    1.00  2290.54     0.00     8.00    23.77    0.04    0.04    0.00   0.00 101.80


01:01:38 DEBUG (ts: 1544374898.817211) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586776.00    0.00  2292.09     0.00     8.00    23.59    0.04    0.04    0.00   0.00 102.00


01:01:39 DEBUG (ts: 1544374899.817229) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586670.00    0.00  2291.68     0.00     8.00    23.48    0.04    0.04    0.00   0.00 101.90


01:01:40 DEBUG (ts: 1544374900.817243) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586606.00    0.00  2291.43     0.00     8.00    23.68    0.04    0.04    0.00   0.00 102.20


01:01:40 DEBUG (ts: 1544374900.918062) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:36|  8  12  61  19   0   0|2291M   24k| 586k 5.00 |42.6 29.9 29.1|9096M 17.9M  142M  243G|   0     0 |1099k 1775k| 1920  23775
10-12 01:01:37|  8  12  61  19   0   0|2292M    0 | 587k    0 |42.6 29.9 29.1|9107M 17.9M  142M  243G|   0     0 |1100k 1776k| 1920  23775
10-12 01:01:38|  9  12  61  19   0   0|2291M   84k| 587k 3.00 |41.7 29.9 29.1|9120M 17.9M  142M  243G|   0     0 |1099k 1775k| 1920  23776
10-12 01:01:39|  9  12  60  19   0   0|2291M    0 | 587k    0 |41.7 29.9 29.1|9130M 17.9M  142M  243G|   0     0 |1099k 1774k| 1920  23776
10-12 01:01:40|  8  12  61  19   0   0|2292M   92k| 587k 4.00 |41.7 29.9 29.1|9139M 17.9M  142M  243G|   0     0 |1099k 1775k| 1920  23776

01:01:41 DEBUG (ts: 1544374901.817271) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    13.00    0.00    2.00     0.00     0.06    60.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586624.00    0.00  2291.50     0.00     8.00    23.58    0.04    0.04    0.00   0.00 102.20


01:01:42 DEBUG (ts: 1544374902.817278) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    5.00     0.00     0.04    17.60     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586709.00    1.00  2291.83     0.00     8.00    23.34    0.04    0.04    0.00   0.00 101.50


01:01:43 DEBUG (ts: 1544374903.817295) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586552.00    0.00  2291.23     0.00     8.00    23.77    0.04    0.04    0.00   0.00 102.60


01:01:44 DEBUG (ts: 1544374904.817306) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586724.00    0.00  2291.88     0.00     8.00    23.56    0.04    0.04    0.00   0.00 101.90


01:01:45 DEBUG (ts: 1544374905.817317) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586747.00    0.00  2291.98     0.00     8.00    23.46    0.04    0.04    0.00   0.00 102.20


01:01:45 DEBUG (ts: 1544374905.918004) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:41|  9  12  61  19   0   0|2291M   16k| 587k 4.00 |41.7 29.9 29.1|9150M 17.9M  142M  243G|   0     0 |1103k 1774k| 1920  23785
10-12 01:01:42|  8  12  61  19   0   0|2291M    0 | 587k    0 |41.7 29.9 29.1|9156M 17.9M  143M  243G|   0     0 |1103k 1774k| 1920  23771
10-12 01:01:43|  8  12  61  19   0   0|2292M   20k| 587k 2.00 |41.7 29.9 29.1|9165M 18.0M  143M  243G|   0     0 |1100k 1775k| 1920  23771
10-12 01:01:44|  8  12  61  19   0   0|2292M    0 | 587k    0 |41.2 30.0 29.2|9173M 18.0M  143M  242G|   0     0 |1099k 1775k| 1920  23771
10-12 01:01:45|  9  12  61  19   0   0|2291M    0 | 586k    0 |41.2 30.0 29.2|9180M 18.0M  143M  242G|   0     0 |1100k 1773k| 1920  23771

01:01:46 DEBUG (ts: 1544374906.817345) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586537.00    0.00  2291.17     0.00     8.00    23.46    0.04    0.04    0.00   0.00 101.40


01:01:47 DEBUG (ts: 1544374907.817381) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    1.00    6.00     0.00     0.06    18.29     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586706.00    0.00  2291.82     0.00     8.00    23.53    0.04    0.04    0.00   0.00 101.90


01:01:48 DEBUG (ts: 1544374908.817326) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586937.00    0.00  2292.72     0.00     8.00    23.67    0.04    0.04    0.00   0.00 101.90


01:01:49 DEBUG (ts: 1544374909.817396) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.01     0.00    16.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586747.00    0.00  2291.98     0.00     8.00    23.89    0.04    0.04    0.00   0.00 102.30


01:01:50 DEBUG (ts: 1544374910.817434) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586608.00    0.00  2291.44     0.00     8.00    23.45    0.04    0.04    0.00   0.00 101.90


01:01:50 DEBUG (ts: 1544374910.918072) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:46|  8  12  61  19   0   0|2292M   60k| 587k 6.00 |41.2 30.0 29.2|9187M 18.0M  143M  242G|   0     0 |1101k 1775k| 1920  23771
10-12 01:01:47|  9  12  60  19   0   0|2292M   28k| 587k 2.00 |41.2 30.0 29.2|9192M 18.0M  143M  242G|   0     0 |1096k 1777k| 1920  23767
10-12 01:01:48|  9  12  60  19   0   0|2292M   16k| 587k 4.00 |41.2 30.0 29.2|9199M 18.0M  143M  242G|   0     0 |1099k 1777k| 1920  23790
10-12 01:01:49|  8  12  61  19   0   0|2291M    0 | 587k    0 |41.2 30.0 29.2|9205M 18.0M  143M  242G|   0     0 |1100k 1774k| 1920  23768
10-12 01:01:50|  8  12  61  19   0   0|2291M    0 | 587k    0 |40.6 30.0 29.2|9210M 18.0M  143M  242G|   0     0 |1103k 1774k| 1920  23768

01:01:51 DEBUG (ts: 1544374911.817462) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586591.00    0.00  2291.37     0.00     8.00    23.19    0.04    0.04    0.00   0.00 101.20


01:01:52 DEBUG (ts: 1544374912.817472) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00   95.00     0.00     0.43     9.35     0.03    0.26    0.00    0.26   0.01   0.10
nvme0n1           0.00     0.00 587048.00    0.00  2293.16     0.00     8.00    23.30    0.04    0.04    0.00   0.00 101.10


01:01:53 DEBUG (ts: 1544374913.817477) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    0.00    2.00     0.00     0.05    48.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586633.00    0.00  2291.54     0.00     8.00    23.55    0.04    0.04    0.00   0.00 101.70


01:01:54 DEBUG (ts: 1544374914.817493) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    0.00    2.00     0.00     0.05    48.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586702.00    0.00  2291.79     0.00     8.00    23.30    0.04    0.04    0.00   0.00 101.70


01:01:55 DEBUG (ts: 1544374915.817508) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    0.00     0.00     0.00     8.00     0.00    1.00    1.00    0.00   1.00   0.10
nvme0n1           0.00     0.00 586456.00    0.00  2290.87     0.00     8.00    23.68    0.04    0.04    0.00   0.00 102.70


01:01:55 DEBUG (ts: 1544374915.918111) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:51|  8  12  61  19   0   0|2293M  444k| 587k 95.0 |40.6 30.0 29.2|9213M 18.0M  143M  242G|   0     0 |1106k 1774k| 1920  23768
10-12 01:01:52|  8  12  61  19   0   0|2292M   48k| 587k 2.00 |40.6 30.0 29.2|9217M 18.0M  143M  242G|   0     0 |1099k 1775k| 1920  23768
10-12 01:01:53|  8  12  61  19   0   0|2292M   48k| 587k 2.00 |40.6 30.0 29.2|9221M 18.0M  143M  242G|   0     0 |1105k 1773k| 1920  23764
10-12 01:01:54|  9  12  60  19   0   0|2291M    0 | 586k    0 |40.6 30.0 29.2|9219M 18.0M  143M  242G|   0     0 |1102k 1774k| 1792  23776
10-12 01:01:55|  9  12  61  19   0   0|2291M    0 | 587k    0 |40.6 30.0 29.2|9222M 18.0M  143M  242G|   0     0 |1103k 1773k| 1792  23765

01:01:56 DEBUG (ts: 1544374916.817533) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586504.00    0.00  2291.03     0.00     8.00    23.53    0.04    0.04    0.00   0.00 102.20


01:01:57 DEBUG (ts: 1544374917.817500) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00   19.00     0.00     0.11    11.37     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586692.00    0.00  2291.77     0.00     8.00    23.72    0.04    0.04    0.00   0.00 102.40


01:01:58 DEBUG (ts: 1544374918.817563) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    2.00    0.00     0.01     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586656.00    0.00  2291.62     0.00     8.00    23.82    0.04    0.04    0.00   0.00 102.80


01:01:59 DEBUG (ts: 1544374919.817564) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    1.00    3.00     0.00     0.05    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586742.00    0.00  2291.96     0.00     8.00    23.36    0.04    0.04    0.00   0.00 102.10


01:02:00 DEBUG (ts: 1544374920.817533) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586841.00    0.00  2292.34     0.00     8.00    23.59    0.04    0.04    0.00   0.00 102.00


01:02:00 DEBUG (ts: 1544374920.917950) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:01:56|  8  12  61  19   0   0|2291M  108k| 587k 19.0 |39.6 30.0 29.2|9225M 18.0M  143M  242G|   0     0 |1100k 1775k| 1792  23765
10-12 01:01:57|  8  12  61  19   0   0|2292M    0 | 587k    0 |39.6 30.0 29.2|9262M 18.0M  143M  242G|   0     0 |1105k 1775k| 1920  23768
10-12 01:01:58|  8  12  61  19   0   0|2292M   52k| 587k 3.00 |39.6 30.0 29.2|9258M 18.0M  143M  242G|   0     0 |1116k 1774k| 1920  23768
10-12 01:01:59|  8  12  61  19   0   0|2292M   32k| 587k 2.00 |39.6 30.0 29.2|9259M 18.0M  143M  242G|   0     0 |1141k 1774k| 1920  23768
10-12 01:02:00|  8  12  60  19   0   0|2291M 8192B| 587k 2.00 |39.6 30.0 29.2|9237M 18.0M  143M  242G|   0     0 |1101k 1776k| 1792  23791

01:02:01 DEBUG (ts: 1544374921.817600) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586634.00    0.00  2291.54     0.00     8.00    23.34    0.04    0.04    0.00   0.00 101.60


01:02:02 DEBUG (ts: 1544374922.817622) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     1.00    0.00   20.00     0.00     0.08     8.40     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586673.00    0.00  2291.69     0.00     8.00    23.44    0.04    0.04    0.00   0.00 102.60


01:02:03 DEBUG (ts: 1544374923.817643) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00   52.00    0.00     0.23     0.00     8.92     0.06    1.06    1.06    0.00   0.12   0.60
nvme0n1           0.00     0.00 586748.00    0.00  2291.98     0.00     8.00    23.55    0.04    0.04    0.00   0.00 102.50


01:02:04 INFO  (ts: 1544374924.225304) engine_race.cc:421: Read Stat of tid 62, elapsed time: 55.419 s, ts: 1544374924.225 s
01:02:04 INFO  (ts: 1544374924.481171) engine_race.cc:421: Read Stat of tid 59, elapsed time: 55.675 s, ts: 1544374924.481 s
01:02:04 DEBUG (ts: 1544374924.817667) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    23.00    0.00    2.00     0.00     0.10   100.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 587037.00    0.00  2293.11     0.00     8.00    23.70    0.04    0.04    0.00   0.00 102.20


01:02:05 DEBUG (ts: 1544374925.817704) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586697.00    0.00  2291.79     0.00     8.00    23.78    0.04    0.04    0.00   0.00 102.30


01:02:05 DEBUG (ts: 1544374925.918060) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:01|  9  12  60  19   0   0|2292M   80k| 587k 19.0 |39.2 30.1 29.2|9238M 18.0M  143M  242G|   0     0 |1102k 1775k| 1792  23768
10-12 01:02:02|  9  12  60  19   0   0|2292M    0 | 587k    0 |39.2 30.1 29.2|9239M 18.2M  143M  242G|   0     0 |1096k 1777k| 1792  23775
10-12 01:02:03|  8  12  60  19   0   0|2293M  100k| 587k 2.00 |39.2 30.1 29.2|9239M 18.2M  143M  242G|   0     0 |1097k 1778k| 1792  23775
10-12 01:02:04|  8  12  61  19   0   0|2292M    0 | 587k    0 |39.2 30.1 29.2|9239M 18.2M  143M  242G|   0     0 |1085k 1755k| 1792  23775
10-12 01:02:05|  8  12  61  19   0   0|2291M   36k| 587k 2.00 |39.2 30.1 29.2|9239M 18.2M  143M  242G|   0     0 |1074k 1740k| 1792  23771

01:02:06 DEBUG (ts: 1544374926.817736) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     7.00    0.00    2.00     0.00     0.04    36.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586579.00    0.00  2291.32     0.00     8.00    23.43    0.04    0.04    0.00   0.00 101.10


01:02:07 DEBUG (ts: 1544374927.817736) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586877.00    0.00  2292.49     0.00     8.00    23.57    0.04    0.04    0.00   0.00 101.90


01:02:08 DEBUG (ts: 1544374928.817743) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586926.00    0.00  2292.68     0.00     8.00    23.62    0.04    0.04    0.00   0.00 101.90


01:02:09 DEBUG (ts: 1544374929.817773) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586656.00    0.00  2291.62     0.00     8.00    23.68    0.04    0.04    0.00   0.00 101.90


01:02:10 DEBUG (ts: 1544374930.817785) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586502.00    0.00  2291.03     0.00     8.00    23.76    0.04    0.04    0.00   0.00 102.40


01:02:10 DEBUG (ts: 1544374930.918029) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:06|  8  12  61  19   0   0|2292M 4096B| 587k 1.00 |38.6 30.1 29.2|9238M 18.2M  143M  242G|   0     0 |1076k 1742k| 1920  23782
10-12 01:02:07|  8  12  61  19   0   0|2293M    0 | 587k    0 |38.6 30.1 29.2|9240M 18.2M  143M  242G|   0     0 |1076k 1741k| 1920  23771
10-12 01:02:08|  8  12  61  19   0   0|2292M   32k| 587k 2.00 |38.6 30.1 29.2|9240M 18.2M  143M  242G|   0     0 |1075k 1740k| 1920  23771
10-12 01:02:09|  8  12  61  19   0   0|2291M    0 | 586k    0 |38.6 30.1 29.2|9240M 18.2M  143M  242G|   0     0 |1075k 1740k| 1920  23771
10-12 01:02:10|  8  12  61  19   0   0|2293M    0 | 587k    0 |38.0 30.1 29.2|9247M 18.2M  143M  242G|   0     0 |1077k 1741k| 1920  23772

01:02:11 DEBUG (ts: 1544374931.817806) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586896.00    0.00  2292.55     0.00     8.00    23.64    0.04    0.04    0.00   0.00 101.90


01:02:12 DEBUG (ts: 1544374932.817831) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    3.00     0.00     0.04    24.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586584.00    0.00  2291.34     0.00     8.00    23.63    0.04    0.04    0.00   0.00 101.70


01:02:13 DEBUG (ts: 1544374933.817848) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586664.00    0.00  2291.66     0.00     8.00    23.63    0.04    0.04    0.00   0.00 101.90


01:02:14 DEBUG (ts: 1544374934.817859) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586920.00    0.00  2292.66     0.00     8.00    24.02    0.04    0.04    0.00   0.00 102.10


01:02:15 DEBUG (ts: 1544374935.817872) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     3.00    0.00    2.00     0.00     0.02    20.00     0.00    0.50    0.00    0.50   0.50   0.10
nvme0n1           0.00     0.00 586790.00    0.00  2292.15     0.00     8.00    23.73    0.04    0.04    0.00   0.00 102.40


01:02:15 DEBUG (ts: 1544374935.918093) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:11|  8  12  61  19   0   0|2291M   36k| 587k 3.00 |38.0 30.1 29.2|9247M 18.2M  143M  242G|   0     0 |1075k 1740k| 1920  23772
10-12 01:02:12|  8  12  61  19   0   0|2292M    0 | 587k    0 |38.0 30.1 29.2|9241M 18.2M  143M  242G|   0     0 |1077k 1741k| 1920  23783
10-12 01:02:13|  8  12  61  19   0   0|2293M    0 | 587k    0 |37.8 30.2 29.3|9239M 18.2M  143M  242G|   0     0 |1077k 1741k| 1920  23783
10-12 01:02:14|  8  12  61  19   0   0|2292M   20k| 587k 2.00 |37.8 30.2 29.3|9240M 18.2M  143M  242G|   0     0 |1075k 1741k| 1920  23772
10-12 01:02:15|  8  12  61  19   0   0|2292M    0 | 587k    0 |37.8 30.2 29.3|9240M 18.2M  143M  242G|   0     0 |1075k 1740k| 1920  23772

01:02:16 DEBUG (ts: 1544374936.817887) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586724.00    0.00  2291.89     0.00     8.00    23.66    0.04    0.04    0.00   0.00 101.70


01:02:17 DEBUG (ts: 1544374937.817905) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    2.00    4.00     0.01     0.02     9.33     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586693.00    0.00  2291.77     0.00     8.00    23.47    0.04    0.04    0.00   0.00 101.90


01:02:18 DEBUG (ts: 1544374938.817925) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586597.00    0.00  2291.39     0.00     8.00    23.59    0.04    0.04    0.00   0.00 101.80


01:02:19 DEBUG (ts: 1544374939.817930) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586806.00    0.00  2292.21     0.00     8.00    23.90    0.04    0.04    0.00   0.00 101.90


01:02:20 DEBUG (ts: 1544374940.817943) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586931.00    0.00  2292.70     0.00     8.00    23.75    0.04    0.04    0.00   0.00 102.60


01:02:20 DEBUG (ts: 1544374940.918057) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:16|  8  12  61  19   0   0|2291M   20k| 587k 4.00 |37.8 30.2 29.3|9240M 18.2M  143M  242G|   0     0 |1076k 1740k| 1920  23783
10-12 01:02:17|  8  12  61  19   0   0|2292M   28k| 587k 2.00 |37.4 30.2 29.3|9240M 18.2M  143M  242G|   0     0 |1075k 1741k| 1920  23772
10-12 01:02:18|  8  12  61  19   0   0|2292M    0 | 587k    0 |37.4 30.2 29.3|9240M 18.2M  143M  242G|   0     0 |1077k 1741k| 1920  23783
10-12 01:02:19|  8  12  61  19   0   0|2293M    0 | 587k    0 |37.4 30.2 29.3|9240M 18.2M  143M  242G|   0     0 |1078k 1741k| 1920  23772
10-12 01:02:20|  8  12  61  19   0   0|2292M   48k| 587k 2.00 |37.4 30.2 29.3|9239M 18.2M  143M  242G|   0     0 |1078k 1741k| 1920  23772

01:02:21 DEBUG (ts: 1544374941.817963) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    0.00    2.00     0.00     0.05    48.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586946.00    0.00  2292.75     0.00     8.00    23.75    0.04    0.04    0.00   0.00 102.20


01:02:22 DEBUG (ts: 1544374942.817974) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    1.00    2.00     0.00     0.01     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586576.00    0.00  2291.32     0.00     8.00    23.87    0.04    0.04    0.00   0.00 102.40


01:02:23 DEBUG (ts: 1544374943.818004) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586808.00    0.00  2292.22     0.00     8.00    23.82    0.04    0.04    0.00   0.00 102.50


01:02:24 DEBUG (ts: 1544374944.818017) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586783.00    0.00  2292.12     0.00     8.00    23.71    0.04    0.04    0.00   0.00 102.00


01:02:25 DEBUG (ts: 1544374945.818033) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586509.00    0.00  2291.05     0.00     8.00    23.89    0.04    0.04    0.00   0.00 102.90


01:02:25 DEBUG (ts: 1544374945.918094) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:21|  8  12  61  19   0   0|2292M 8192B| 587k 2.00 |37.4 30.2 29.3|9239M 18.2M  143M  242G|   0     0 |1075k 1740k| 1920  23772
10-12 01:02:22|  8  12  61  19   0   0|2292M    0 | 587k    0 |37.0 30.3 29.3|9239M 18.2M  143M  242G|   0     0 |1077k 1740k| 1920  23772
10-12 01:02:23|  8  12  61  19   0   0|2292M   28k| 587k 2.00 |37.0 30.3 29.3|9239M 18.3M  143M  242G|   0     0 |1076k 1741k| 1920  23772
10-12 01:02:24|  8  12  61  19   0   0|2291M    0 | 587k    0 |37.0 30.3 29.3|9239M 18.3M  143M  242G|   0     0 |1077k 1740k| 1920  23783
10-12 01:02:25|  8  12  61  19   0   0|2292M   32k| 587k 2.00 |37.0 30.3 29.3|9239M 18.3M  143M  242G|   0     0 |1076k 1740k| 1920  23772

01:02:26 DEBUG (ts: 1544374946.818055) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586655.00    0.00  2291.62     0.00     8.00    23.74    0.04    0.04    0.00   0.00 101.50


01:02:27 DEBUG (ts: 1544374947.818072) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00  115.00     0.00     0.51     9.04     0.04    0.38    0.00    0.38   0.01   0.10
nvme0n1           0.00     0.00 586875.00    0.00  2292.48     0.00     8.00    23.70    0.04    0.04    0.00   0.00 102.70


01:02:28 DEBUG (ts: 1544374948.818079) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586544.00    0.00  2291.19     0.00     8.00    23.60    0.04    0.04    0.00   0.00 101.40


01:02:29 DEBUG (ts: 1544374949.818130) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586477.00    0.00  2290.93     0.00     8.00    23.36    0.04    0.04    0.00   0.00 101.80


01:02:30 DEBUG (ts: 1544374950.818099) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    0.00    2.00     0.00     0.05    48.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586878.00    0.00  2292.49     0.00     8.00    23.73    0.04    0.04    0.00   0.00 102.90


01:02:30 DEBUG (ts: 1544374950.918023) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:26|  8  12  61  19   0   0|2292M  520k| 587k  115 |36.7 30.3 29.3|9239M 18.3M  143M  242G|   0     0 |1077k 1741k| 1920  23772
10-12 01:02:27|  8  12  61  19   0   0|2291M    0 | 587k    0 |36.7 30.3 29.3|9239M 18.3M  143M  242G|   0     0 |1076k 1740k| 1920  23772
10-12 01:02:28|  8  12  61  19   0   0|2291M 4096B| 587k 1.00 |36.7 30.3 29.3|9235M 18.3M  143M  242G|   0     0 |1076k 1739k| 1920  23772
10-12 01:02:29|  8  12  61  19   0   0|2292M   48k| 587k 2.00 |36.7 30.3 29.3|9235M 18.3M  143M  242G|   0     0 |1077k 1741k| 1920  23772
10-12 01:02:30|  8  12  61  19   0   0|2292M   56k| 587k 4.00 |36.7 30.3 29.3|9237M 18.3M  143M  242G|   0     0 |1079k 1742k| 1792  23807

01:02:31 DEBUG (ts: 1544374951.818158) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    10.00    0.00    3.00     0.00     0.05    34.67     0.00    0.33    0.00    0.33   0.33   0.10
nvme0n1           0.00     0.00 586828.00    0.00  2292.30     0.00     8.00    23.66    0.04    0.04    0.00   0.00 101.60


01:02:32 DEBUG (ts: 1544374952.818155) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     2.00    2.00   14.00     0.01     0.08    11.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586755.00    0.00  2292.01     0.00     8.00    23.55    0.04    0.04    0.00   0.00 102.30


01:02:33 DEBUG (ts: 1544374953.818148) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586500.00    0.00  2291.01     0.00     8.00    23.76    0.04    0.04    0.00   0.00 102.70


01:02:34 DEBUG (ts: 1544374954.818169) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586360.00    0.00  2290.47     0.00     8.00    24.01    0.04    0.04    0.00   0.00 102.90


01:02:35 DEBUG (ts: 1544374955.818195) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586573.00    0.00  2291.30     0.00     8.00    23.85    0.04    0.04    0.00   0.00 102.00


01:02:35 DEBUG (ts: 1544374955.918072) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:31|  8  12  61  19   0   0|2292M   76k| 587k 13.0 |36.5 30.4 29.4|9235M 18.3M  143M  242G|   0     0 |1077k 1741k| 1792  23772
10-12 01:02:32|  8  12  61  19   0   0|2291M    0 | 586k    0 |36.5 30.4 29.4|9245M 18.3M  143M  242G|   0     0 |1077k 1740k| 1920  23773
10-12 01:02:33|  8  12  61  19   0   0|2291M    0 | 586k    0 |36.5 30.4 29.4|9245M 18.3M  143M  242G|   0     0 |1076k 1740k| 1920  23773
10-12 01:02:34|  8  12  61  19   0   0|2291M    0 | 587k    0 |36.5 30.4 29.4|9245M 18.3M  143M  242G|   0     0 |1076k 1740k| 1920  23773
10-12 01:02:35|  8  12  61  19   0   0|2291M  152k| 587k 4.00 |36.5 30.4 29.4|9244M 18.3M  143M  242G|   0     0 |1077k 1740k| 1792  23773

01:02:36 DEBUG (ts: 1544374956.818208) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00    34.00    0.00    4.00     0.00     0.15    76.00     0.00    0.25    0.00    0.25   0.25   0.10
nvme0n1           0.00     0.00 586569.00    0.00  2291.29     0.00     8.00    23.75    0.04    0.04    0.00   0.00 102.40


01:02:37 DEBUG (ts: 1544374957.818224) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00   14.00     0.00     0.05     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586556.00    0.00  2291.23     0.00     8.00    23.62    0.04    0.04    0.00   0.00 101.70


01:02:38 DEBUG (ts: 1544374958.818236) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586769.00    0.00  2292.07     0.00     8.00    23.80    0.04    0.04    0.00   0.00 102.20


01:02:39 DEBUG (ts: 1544374959.818251) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586655.00    0.00  2291.62     0.00     8.00    23.53    0.04    0.04    0.00   0.00 101.70


01:02:40 DEBUG (ts: 1544374960.818265) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586451.00    0.00  2290.82     0.00     8.00    23.73    0.04    0.04    0.00   0.00 102.20


01:02:40 DEBUG (ts: 1544374960.918070) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:36|  8  12  61  19   0   0|2292M   56k| 587k 14.0 |36.5 30.4 29.4|9244M 18.3M  143M  242G|   0     0 |1077k 1741k| 1792  23784
10-12 01:02:37|  8  12  61  19   0   0|2292M    0 | 587k    0 |36.5 30.4 29.4|9244M 18.3M  143M  242G|   0     0 |1077k 1740k| 1792  23773
10-12 01:02:38|  8  12  61  19   0   0|2291M    0 | 587k    0 |35.8 30.4 29.3|9244M 18.3M  143M  242G|   0     0 |1075k 1740k| 1792  23773
10-12 01:02:39|  8  12  61  19   0   0|2291M    0 | 587k    0 |35.8 30.4 29.3|9244M 18.3M  143M  242G|   0     0 |1076k 1740k| 1792  23773
10-12 01:02:40|  8  12  61  19   0   0|2292M   28k| 587k 2.00 |35.8 30.4 29.3|9244M 18.3M  143M  242G|   0     0 |1076k 1740k| 1792  23773

01:02:41 DEBUG (ts: 1544374961.818291) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586738.00    0.00  2291.95     0.00     8.00    23.81    0.04    0.04    0.00   0.00 102.30


01:02:42 DEBUG (ts: 1544374962.818353) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     6.00    0.00    2.00     0.00     0.03    32.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586739.00    0.00  2291.95     0.00     8.00    23.71    0.04    0.04    0.00   0.00 103.00


01:02:43 DEBUG (ts: 1544374963.818357) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    1.00     0.00     0.00     8.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586900.00    0.00  2292.58     0.00     8.00    23.42    0.04    0.04    0.00   0.00 101.60


01:02:44 DEBUG (ts: 1544374964.818365) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586622.00    0.00  2291.49     0.00     8.00    24.09    0.04    0.04    0.00   0.00 104.00


01:02:45 DEBUG (ts: 1544374965.818371) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586702.00    0.00  2291.80     0.00     8.00    23.73    0.04    0.04    0.00   0.00 101.50


01:02:45 DEBUG (ts: 1544374965.917942) util.h:67:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:41|  8  12  61  19   0   0|2292M   36k| 587k 3.00 |35.8 30.4 29.3|9246M 18.3M  143M  242G|   0     0 |1079k 1741k| 1792  23773
10-12 01:02:42|  8  12  61  19   0   0|2292M    0 | 587k    0 |35.8 30.4 29.3|9244M 18.3M  143M  242G|   0     0 |1084k 1742k| 1792  23784
10-12 01:02:43|  8  12  61  19   0   0|2292M    0 | 587k    0 |35.8 30.4 29.3|9244M 18.3M  143M  242G|   0     0 |1076k 1740k| 1792  23769
10-12 01:02:44|  8  12  61  19   0   0|2291M    0 | 587k    0 |35.5 30.4 29.4|9244M 18.3M  143M  242G|   0     0 |1076k 1740k| 1792  23769
10-12 01:02:45|  8  12  61  19   0   0|2292M    0 | 587k    0 |35.5 30.4 29.4|9239M 18.3M  143M  242G|   0     0 |1077k 1741k| 1792  23765

01:02:46 DEBUG (ts: 1544374966.818372) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586718.00    0.00  2291.86     0.00     8.00    23.64    0.04    0.04    0.00   0.00 102.10


01:02:46 DEBUG (ts: 1544374966.918064) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:46|  8  12  61  19   0   0|2292M   24k| 587k 2.00 |35.5 30.4 29.4|9239M 18.3M  143M  242G|   0     0 |1077k 1740k| 1792  23765

01:02:47 DEBUG (ts: 1544374967.818392) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     4.00    0.00    2.00     0.00     0.02    24.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586593.00    0.00  2291.38     0.00     8.00    23.59    0.04    0.04    0.00   0.00 102.30


01:02:47 DEBUG (ts: 1544374967.917941) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:47|  8  12  61  19   0   0|2292M   28k| 587k 2.00 |35.5 30.4 29.4|9239M 18.3M  143M  242G|   0     0 |1076k 1741k| 1792  23765

01:02:48 DEBUG (ts: 1544374968.818404) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     5.00    0.00    2.00     0.00     0.03    28.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586794.00    0.00  2292.16     0.00     8.00    23.68    0.04    0.04    0.00   0.00 102.00


01:02:48 DEBUG (ts: 1544374968.918041) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:48|  8  12  61  19   0   0|2292M    0 | 587k    0 |35.5 30.4 29.4|9238M 18.3M  143M  242G|   0     0 |1076k 1741k| 1792  23787

01:02:49 DEBUG (ts: 1544374969.818414) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586588.00    0.00  2291.36     0.00     8.00    23.65    0.04    0.04    0.00   0.00 102.20


01:02:49 DEBUG (ts: 1544374969.917954) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:49|  8  11  61  19   0   0|2292M    0 | 587k    0 |35.5 30.4 29.4|9239M 18.3M  143M  242G|   0     0 |1074k 1741k| 1792  23765

01:02:50 DEBUG (ts: 1544374970.818420) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586857.00    0.00  2292.41     0.00     8.00    23.82    0.04    0.04    0.00   0.00 102.10


01:02:50 DEBUG (ts: 1544374970.917954) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:50|  8  12  61  19   0   0|2292M    0 | 587k    0 |35.4 30.4 29.4|9239M 18.3M  143M  242G|   0     0 |1073k 1741k| 1792  23761

01:02:51 DEBUG (ts: 1544374971.818458) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586756.00    0.00  2292.02     0.00     8.00    23.76    0.04    0.04    0.00   0.00 102.70


01:02:51 DEBUG (ts: 1544374971.918053) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:51|  8  11  61  19   0   0|2292M    0 | 587k    0 |35.4 30.4 29.4|9239M 18.3M  143M  242G|   0     0 |1074k 1741k| 1792  23761

01:02:52 DEBUG (ts: 1544374972.818469) util.h:109:
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00 586652.00    0.00  2291.61     0.00     8.00    23.91    0.04    0.04    0.00   0.00 101.60


01:02:52 DEBUG (ts: 1544374972.918124) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:52|  8  12  61  19   0   0|2292M   32k| 587k 2.00 |35.4 30.4 29.4|9254M 18.3M  143M  242G|   0     0 |1074k 1742k| 1792  23756

01:02:53 DEBUG (ts: 1544374973.918116) util.h:77:
----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg--- ------memory-usage----- ---paging-- ---system-- --filesystem-
     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | used  buff  cach  free|  in   out | int   csw |files  inodes
10-12 01:02:53|  8  12  61  19   0   0|2291M   28k| 586k 2.00 |35.4 30.4 29.4|9254M 18.3M  143M  242G|   0     0 |1073k 1740k| 1792  23756

01:02:54 INFO  (ts: 1544374974.266121) engine_race.cc:421: Read Stat of tid 8, elapsed time: 105.460 s, ts: 1544374974.266 s
01:02:54 INFO  (ts: 1544374974.315138) engine_race.cc:421: Read Stat of tid 51, elapsed time: 105.509 s, ts: 1544374974.315 s
01:02:54 INFO  (ts: 1544374974.378404) engine_race.cc:421: Read Stat of tid 20, elapsed time: 105.572 s, ts: 1544374974.378 s
01:02:54 INFO  (ts: 1544374974.395750) engine_race.cc:421: Read Stat of tid 39, elapsed time: 105.589 s, ts: 1544374974.395 s
01:02:54 INFO  (ts: 1544374974.406067) engine_race.cc:421: Read Stat of tid 58, elapsed time: 105.600 s, ts: 1544374974.406 s
01:02:54 INFO  (ts: 1544374974.411741) engine_race.cc:421: Read Stat of tid 47, elapsed time: 105.605 s, ts: 1544374974.411 s
01:02:54 INFO  (ts: 1544374974.428160) engine_race.cc:421: Read Stat of tid 52, elapsed time: 105.622 s, ts: 1544374974.428 s
01:02:54 INFO  (ts: 1544374974.459811) engine_race.cc:421: Read Stat of tid 35, elapsed time: 105.653 s, ts: 1544374974.459 s
01:02:54 INFO  (ts: 1544374974.463669) engine_race.cc:421: Read Stat of tid 61, elapsed time: 105.657 s, ts: 1544374974.463 s
01:02:54 INFO  (ts: 1544374974.521195) engine_race.cc:421: Read Stat of tid 6, elapsed time: 105.715 s, ts: 1544374974.521 s
01:02:54 INFO  (ts: 1544374974.524583) engine_race.cc:421: Read Stat of tid 4, elapsed time: 105.718 s, ts: 1544374974.524 s
01:02:54 INFO  (ts: 1544374974.535747) engine_race.cc:421: Read Stat of tid 28, elapsed time: 105.729 s, ts: 1544374974.535 s
01:02:54 INFO  (ts: 1544374974.537630) engine_race.cc:421: Read Stat of tid 16, elapsed time: 105.731 s, ts: 1544374974.537 s
01:02:54 INFO  (ts: 1544374974.542184) engine_race.cc:421: Read Stat of tid 27, elapsed time: 105.736 s, ts: 1544374974.542 s
01:02:54 INFO  (ts: 1544374974.569997) engine_race.cc:421: Read Stat of tid 21, elapsed time: 105.764 s, ts: 1544374974.569 s
01:02:54 INFO  (ts: 1544374974.579544) engine_race.cc:421: Read Stat of tid 34, elapsed time: 105.773 s, ts: 1544374974.579 s
01:02:54 INFO  (ts: 1544374974.589492) engine_race.cc:421: Read Stat of tid 17, elapsed time: 105.783 s, ts: 1544374974.589 s
01:02:54 INFO  (ts: 1544374974.593184) engine_race.cc:421: Read Stat of tid 18, elapsed time: 105.787 s, ts: 1544374974.593 s
01:02:54 INFO  (ts: 1544374974.594479) engine_race.cc:421: Read Stat of tid 54, elapsed time: 105.788 s, ts: 1544374974.594 s
01:02:54 INFO  (ts: 1544374974.596863) engine_race.cc:421: Read Stat of tid 14, elapsed time: 105.791 s, ts: 1544374974.596 s
01:02:54 INFO  (ts: 1544374974.597781) engine_race.cc:421: Read Stat of tid 19, elapsed time: 105.791 s, ts: 1544374974.597 s
01:02:54 INFO  (ts: 1544374974.597810) engine_race.cc:421: Read Stat of tid 29, elapsed time: 105.791 s, ts: 1544374974.597 s
01:02:54 INFO  (ts: 1544374974.599243) engine_race.cc:421: Read Stat of tid 1, elapsed time: 105.793 s, ts: 1544374974.599 s
01:02:54 INFO  (ts: 1544374974.602490) engine_race.cc:421: Read Stat of tid 46, elapsed time: 105.796 s, ts: 1544374974.602 s
01:02:54 INFO  (ts: 1544374974.603740) engine_race.cc:421: Read Stat of tid 23, elapsed time: 105.797 s, ts: 1544374974.603 s
01:02:54 INFO  (ts: 1544374974.603741) engine_race.cc:421: Read Stat of tid 3, elapsed time: 105.797 s, ts: 1544374974.603 s
01:02:54 INFO  (ts: 1544374974.605898) engine_race.cc:421: Read Stat of tid 25, elapsed time: 105.800 s, ts: 1544374974.605 s
01:02:54 INFO  (ts: 1544374974.605931) engine_race.cc:421: Read Stat of tid 24, elapsed time: 105.800 s, ts: 1544374974.605 s
01:02:54 INFO  (ts: 1544374974.606930) engine_race.cc:421: Read Stat of tid 60, elapsed time: 105.801 s, ts: 1544374974.606 s
01:02:54 INFO  (ts: 1544374974.609869) engine_race.cc:421: Read Stat of tid 0, elapsed time: 105.804 s, ts: 1544374974.609 s
01:02:54 INFO  (ts: 1544374974.616841) engine_race.cc:421: Read Stat of tid 15, elapsed time: 105.810 s, ts: 1544374974.616 s
01:02:54 INFO  (ts: 1544374974.618318) engine_race.cc:421: Read Stat of tid 42, elapsed time: 105.812 s, ts: 1544374974.618 s
01:02:54 INFO  (ts: 1544374974.621217) engine_race.cc:421: Read Stat of tid 37, elapsed time: 105.815 s, ts: 1544374974.621 s
01:02:54 INFO  (ts: 1544374974.626305) engine_race.cc:421: Read Stat of tid 2, elapsed time: 105.820 s, ts: 1544374974.626 s
01:02:54 INFO  (ts: 1544374974.627873) engine_race.cc:421: Read Stat of tid 9, elapsed time: 105.822 s, ts: 1544374974.627 s
01:02:54 INFO  (ts: 1544374974.629535) engine_race.cc:421: Read Stat of tid 40, elapsed time: 105.823 s, ts: 1544374974.629 s
01:02:54 INFO  (ts: 1544374974.633778) engine_race.cc:421: Read Stat of tid 10, elapsed time: 105.827 s, ts: 1544374974.633 s
01:02:54 INFO  (ts: 1544374974.637382) engine_race.cc:421: Read Stat of tid 13, elapsed time: 105.831 s, ts: 1544374974.637 s
01:02:54 INFO  (ts: 1544374974.642336) engine_race.cc:421: Read Stat of tid 30, elapsed time: 105.836 s, ts: 1544374974.642 s
01:02:54 INFO  (ts: 1544374974.647773) engine_race.cc:421: Read Stat of tid 36, elapsed time: 105.841 s, ts: 1544374974.647 s
01:02:54 INFO  (ts: 1544374974.649620) engine_race.cc:421: Read Stat of tid 38, elapsed time: 105.843 s, ts: 1544374974.649 s
01:02:54 INFO  (ts: 1544374974.650464) engine_race.cc:421: Read Stat of tid 33, elapsed time: 105.844 s, ts: 1544374974.650 s
01:02:54 INFO  (ts: 1544374974.656308) engine_race.cc:421: Read Stat of tid 43, elapsed time: 105.850 s, ts: 1544374974.656 s
01:02:54 INFO  (ts: 1544374974.660035) engine_race.cc:421: Read Stat of tid 45, elapsed time: 105.854 s, ts: 1544374974.660 s
01:02:54 INFO  (ts: 1544374974.670537) engine_race.cc:421: Read Stat of tid 53, elapsed time: 105.864 s, ts: 1544374974.670 s
01:02:54 INFO  (ts: 1544374974.677152) engine_race.cc:421: Read Stat of tid 26, elapsed time: 105.871 s, ts: 1544374974.677 s
01:02:54 INFO  (ts: 1544374974.678047) engine_race.cc:421: Read Stat of tid 41, elapsed time: 105.872 s, ts: 1544374974.678 s
01:02:54 INFO  (ts: 1544374974.682047) engine_race.cc:421: Read Stat of tid 63, elapsed time: 105.876 s, ts: 1544374974.682 s
01:02:54 INFO  (ts: 1544374974.689779) engine_race.cc:421: Read Stat of tid 44, elapsed time: 105.883 s, ts: 1544374974.689 s
01:02:54 INFO  (ts: 1544374974.690044) engine_race.cc:421: Read Stat of tid 32, elapsed time: 105.884 s, ts: 1544374974.690 s
01:02:54 INFO  (ts: 1544374974.703339) engine_race.cc:421: Read Stat of tid 50, elapsed time: 105.897 s, ts: 1544374974.703 s
01:02:54 INFO  (ts: 1544374974.708410) engine_race.cc:421: Read Stat of tid 31, elapsed time: 105.902 s, ts: 1544374974.708 s
01:02:54 INFO  (ts: 1544374974.710902) engine_race.cc:421: Read Stat of tid 7, elapsed time: 105.905 s, ts: 1544374974.710 s
01:02:54 INFO  (ts: 1544374974.721275) engine_race.cc:421: Read Stat of tid 48, elapsed time: 105.915 s, ts: 1544374974.721 s
01:02:54 INFO  (ts: 1544374974.723818) engine_race.cc:421: Read Stat of tid 11, elapsed time: 105.917 s, ts: 1544374974.723 s
01:02:54 INFO  (ts: 1544374974.725640) engine_race.cc:421: Read Stat of tid 5, elapsed time: 105.919 s, ts: 1544374974.725 s
01:02:54 INFO  (ts: 1544374974.727193) engine_race.cc:421: Read Stat of tid 57, elapsed time: 105.921 s, ts: 1544374974.727 s
01:02:54 INFO  (ts: 1544374974.731950) engine_race.cc:421: Read Stat of tid 49, elapsed time: 105.926 s, ts: 1544374974.731 s
01:02:54 INFO  (ts: 1544374974.731957) engine_race.cc:421: Read Stat of tid 56, elapsed time: 105.926 s, ts: 1544374974.731 s
01:02:54 INFO  (ts: 1544374974.734884) engine_race.cc:421: Read Stat of tid 22, elapsed time: 105.929 s, ts: 1544374974.734 s
01:02:54 INFO  (ts: 1544374974.741452) engine_race.cc:421: Read Stat of tid 12, elapsed time: 105.935 s, ts: 1544374974.741 s
01:02:54 INFO  (ts: 1544374974.741486) engine_race.cc:421: Read Stat of tid 55, elapsed time: 105.935 s, ts: 1544374974.741 s
readrandom   :       1.652 micros/op 605369 ops/sec; 2295.3 MB/s (1000000 of 1000000 found)

Microseconds per read:
Count: 64000000 Average: 104.0136  StdDev: 41.93
Min: 0  Median: 68.1727  Max: 11055
Percentiles: P50: 68.17 P75: 138.72 P99: 378.66 P99.9: 573.73 P99.99: 836.51
------------------------------------------------------
[       0,       1 ]      184   0.000%   0.000%
(       1,       2 ]    37250   0.058%   0.058%
(       2,       3 ]   384468   0.601%   0.659%
(       3,       4 ]   381362   0.596%   1.255%
(       4,       6 ]   150006   0.234%   1.489%
(       6,      10 ]    53445   0.084%   1.573%
(      10,      15 ]   251080   0.392%   1.965%
(      15,      22 ]    54379   0.085%   2.050%
(      22,      34 ]    18300   0.029%   2.079%
(      34,      51 ] 12364918  19.320%  21.399% ####
(      51,      76 ] 26647771  41.637%  63.036% ########
(      76,     110 ]  1205915   1.884%  64.920%
(     110,     170 ] 13475408  21.055%  85.976% ####
(     170,     250 ]  4966216   7.760%  93.735% ##
(     250,     380 ]  3404412   5.319%  99.055% #
(     380,     580 ]   558383   0.872%  99.927%
(     580,     870 ]    45339   0.071%  99.998%
(     870,    1300 ]     1097   0.002% 100.000%
(    1300,    1900 ]       54   0.000% 100.000%
(    1900,    2900 ]       10   0.000% 100.000%
(    2900,    4400 ]        2   0.000% 100.000%
(    9900,   14000 ]        1   0.000% 100.000%


------------------------------------------------
!!!Competion Report!!!
         readrandom: 605369 ops/second
disk usage: 278175 MB
------------------------------------------------
01:02:54 (Func: ~EngineRace, Line: 247), (TS: 1544374974.741930 s)-(Elapsed: 105.936075 s), (Mem: 1105.265625 MB)

01:02:54 (Func: ~EngineRace, Line: 310), (TS: 1544374974.742056 s)-(Elapsed: 105.936200 s), (Mem: 1105.492188 MB)

01:02:54 (Func: ~EngineRace, Line: 344), (TS: 1544374974.742133 s)-(Elapsed: 105.936277 s), (Mem: 1105.488281 MB)
```