# 思路

## test-device-write: Code Updated Unitl 11/07 (From 11/04).

控制变量 | 范围
--- | ---
order | 顺序写，随机写(random shuffle)
buffer-size | `{1,2,4,8,16}` `*` `4KB`

* 顺序 vs 随机写 value no-fallocate (2018-11-05_11-40-testwrite-vary-buffsize-seq-rand, not-o-direct)

```
11:16:30 156:[0m Close the database.
11:16:30 412:[0m Close file..
11:16:30 426:[0m 0
11:18:26 352:[0m Step one File exists, 508,500, 116.089
11:18:26 426:[0m 1
11:20:21 352:[0m Step one File exists, 506,536, 114.852
11:20:21 426:[0m 2
11:22:15 352:[0m Step one File exists, 508,584, 114.419
11:22:15 426:[0m 3
11:24:10 352:[0m Step one File exists, 510,688, 114.930
11:24:10 426:[0m 4
11:26:06 352:[0m Step one File exists, 510,824, 115.395
11:26:06 436:[0m Close file end..
11:26:06 457:[0m 0
11:28:02 352:[0m Step one File exists, 513,432, 116.023
11:28:02 457:[0m 1
11:29:57 352:[0m Step one File exists, 513,432, 114.868
11:29:57 457:[0m 2
11:31:51 352:[0m Step one File exists, 513,432, 114.491
11:31:51 457:[0m 3
11:33:46 352:[0m Step one File exists, 513,432, 114.862
11:33:46 457:[0m 4
11:35:41 352:[0m Step one File exists, 513,432, 115.382
11:35:41 185:[0m Close the database successfully.
```

*  顺序写 no-fallocate (2018-11-07_17-55-write-kv-no-fallocate)

```
17:34:27 180:[0m Close the database.
17:34:27 480:[0m Close file..
17:34:27 494:[0m 0
17:34:27 355:[0m rm -r test_directory/*
17:38:26 414:[0m Step one File exists, 508,592, 238.938
17:38:26 426:[0m Step one File exists, 508,592, 238.992 end.
17:40:16 458:[0m Step two File exists, 508,592, 109.900
17:40:16 494:[0m 1
17:40:16 355:[0m rm -r test_directory/*
17:42:12 414:[0m Step one File exists, 506,752, 115.441
17:42:12 426:[0m Step one File exists, 506,752, 115.443 end.
17:44:02 458:[0m Step two File exists, 506,936, 109.898
17:44:02 494:[0m 2
17:44:02 355:[0m rm -r test_directory/*
17:45:56 414:[0m Step one File exists, 506,936, 114.264
17:45:56 426:[0m Step one File exists, 506,936, 114.273 end.
17:47:42 458:[0m Step two File exists, 506,936, 105.959
17:47:42 494:[0m 3
17:47:42 355:[0m rm -r test_directory/*
17:49:37 414:[0m Step one File exists, 506,980, 114.714
17:49:37 426:[0m Step one File exists, 506,980, 114.724 end.
17:51:18 458:[0m Step two File exists, 507,132, 100.984
17:51:18 494:[0m 4
17:51:18 355:[0m rm -r test_directory/*
17:53:14 414:[0m Step one File exists, 507,344, 115.299
17:53:14 426:[0m Step one File exists, 507,344, 115.305 end.
17:54:52 458:[0m Step two File exists, 508,928, 98.516
17:54:52 215:[0m Close the database successfully.
```

* 顺序写 fallocate (2018-11-07_20-15-write-kv-fallocate)

```
20:02:05 180:[0m Close the database.
20:02:05 483:[0m Close file..
20:02:05 497:[0m 0
20:02:05 355:[0m rm -r test_directory/*
20:02:05 377:[0m Release File exists, 508,420, 0.263
20:04:15 417:[0m Step one File exists, 508,576, 130.198
20:04:16 429:[0m Step one File exists, 508,576, 130.241 end.
20:04:16 497:[0m 1
20:04:16 355:[0m rm -r test_directory/*
20:04:16 377:[0m Release File exists, 506,844, 0.183
20:06:10 417:[0m Step one File exists, 507,132, 114.513
20:06:10 429:[0m Step one File exists, 507,132, 114.551 end.
20:06:10 497:[0m 2
20:06:10 355:[0m rm -r test_directory/*
20:06:11 377:[0m Release File exists, 507,148, 0.247
20:08:05 417:[0m Step one File exists, 507,260, 113.964
20:08:05 429:[0m Step one File exists, 507,260, 113.971 end.
20:08:05 497:[0m 3
20:08:05 355:[0m rm -r test_directory/*
20:08:05 377:[0m Release File exists, 507,420, 0.209
20:10:00 417:[0m Step one File exists, 507,668, 114.389
20:10:00 429:[0m Step one File exists, 507,668, 114.393 end.
20:10:00 497:[0m 4
20:10:00 355:[0m rm -r test_directory/*
20:10:00 377:[0m Release File exists, 507,864, 0.268
20:11:55 417:[0m Step one File exists, 508,156, 114.736
20:11:55 429:[0m Step one File exists, 508,156, 114.744 end.
20:11:55 215:[0m Close the database successfully.
```

* 随机和顺序读写 (2018-11-07_01-35-write-odirect-also)

```
00:53:54 :159:[0m Close the database.
00:53:54 :431:[0m Close file..
00:53:54 :445:[0m 0
00:55:49 :368:[0m Step one File exists, 508,516, 115.656
00:55:49 :377:[0m Step one File exists, 508,516, 115.657 end.
00:57:39 :409:[0m Step two File exists, 508,580, 109.911
00:57:39 :445:[0m 1
00:59:34 :368:[0m Step one File exists, 514,656, 114.454
00:59:34 :377:[0m Step one File exists, 514,656, 114.455 end.
01:01:23 :409:[0m Step two File exists, 515,016, 109.804
01:01:23 :445:[0m 2
01:03:18 :368:[0m Step one File exists, 515,016, 114.241
01:03:18 :377:[0m Step one File exists, 515,016, 114.242 end.
01:05:04 :409:[0m Step two File exists, 515,016, 106.102
01:05:04 :445:[0m 3
01:06:58 :368:[0m Step one File exists, 515,036, 114.576
01:06:58 :377:[0m Step one File exists, 515,036, 114.577 end.
01:08:39 :409:[0m Step two File exists, 515,192, 101.067
01:08:39 :445:[0m 4
01:10:35 :368:[0m Step one File exists, 515,308, 115.066
01:10:35 :377:[0m Step one File exists, 515,308, 115.066 end.
01:12:13 :409:[0m Step two File exists, 517,124, 98.652
01:12:13 :455:[0m Close file end..
01:12:13 :476:[0m 0
01:14:09 :368:[0m Step one File exists, 518,108, 115.630
01:14:09 :377:[0m Step one File exists, 518,108, 115.630 end.
01:15:59 :409:[0m Step two File exists, 518,108, 109.734
01:15:59 :476:[0m 1
01:17:53 :368:[0m Step one File exists, 518,108, 114.688
01:17:53 :377:[0m Step one File exists, 518,108, 114.688 end.
01:19:43 :409:[0m Step two File exists, 518,108, 110.020
01:19:43 :476:[0m 2
01:21:38 :368:[0m Step one File exists, 518,108, 114.349
01:21:38 :377:[0m Step one File exists, 518,108, 114.349 end.
01:23:24 :409:[0m Step two File exists, 518,108, 105.950
01:23:24 :476:[0m 3
01:25:18 :368:[0m Step one File exists, 518,108, 114.743
01:25:18 :377:[0m Step one File exists, 518,108, 114.743 end.
01:27:00 :409:[0m Step two File exists, 518,108, 101.057
01:27:00 :476:[0m 4
01:28:55 :368:[0m Step one File exists, 518,108, 115.197
01:28:55 :377:[0m Step one File exists, 518,108, 115.197 end.
01:30:33 :409:[0m Step two File exists, 518,108, 98.599
01:30:33 :194:[0m Close the database successfully.
```

* 随机和顺序总结
    * buffer-size足够大随机和顺序的读写性能相近
    * 顺序和随机对read影响更大, 对write影响相对较小

* write-benchmark总结
    * 16KB buffer写入最好
    * fallocate与否总时间差别不大 (加上`0.263s` fallocate time), `4KB` no fallocate very bad, `8KB` no fallocte ok

* read-benchmark总结
    * read `block size / buffer size` 一次较大更好

### test-device-write-aio: Code Updated Unitl 11/14 (From 11/12).

答辩中不讲。在写入中用处不大。

* Write-Only

变量 | 经验
--- | ---
thread# |  32 ok, at least `4` (better than `2`)
QueueDepth | 32 ok
io_getevents block threshold | `4`, busy waiting cost too much cpu

### Others

small queue-depth is ok for write. (199064, 2018-11-14_11-36)

8 thread per round is ok because in a no-dependency implementation.

```
14:46:40 396: Write Stat of tid 56, mem usage: 576,520 KB, elapsed time: 14.337 s, ts: 1542091600.226 s
14:46:40 396: Write Stat of tid 57, mem usage: 576,768 KB, elapsed time: 14.337 s, ts: 1542091600.227 s
14:46:40 396: Write Stat of tid 58, mem usage: 577,012 KB, elapsed time: 14.337 s, ts: 1542091600.227 s
14:46:40 396: Write Stat of tid 63, mem usage: 577,256 KB, elapsed time: 14.337 s, ts: 1542091600.227 s
14:46:40 396: Write Stat of tid 60, mem usage: 577,500 KB, elapsed time: 14.337 s, ts: 1542091600.227 s
14:46:40 396: Write Stat of tid 59, mem usage: 577,744 KB, elapsed time: 14.338 s, ts: 1542091600.227 s
14:46:40 396: Write Stat of tid 61, mem usage: 577,988 KB, elapsed time: 14.338 s, ts: 1542091600.228 s
14:46:40 396: Write Stat of tid 62, mem usage: 577,988 KB, elapsed time: 14.338 s, ts: 1542091600.228 s
14:46:54 396: Write Stat of tid 53, mem usage: 576,508 KB, elapsed time: 28.705 s, ts: 1542091614.593 s
14:46:54 396: Write Stat of tid 51, mem usage: 576,548 KB, elapsed time: 28.706 s, ts: 1542091614.594 s
14:46:54 396: Write Stat of tid 48, mem usage: 576,800 KB, elapsed time: 28.707 s, ts: 1542091614.595 s
14:46:54 396: Write Stat of tid 54, mem usage: 577,052 KB, elapsed time: 28.707 s, ts: 1542091614.595 s
14:46:54 396: Write Stat of tid 50, mem usage: 577,304 KB, elapsed time: 28.707 s, ts: 1542091614.595 s
14:46:54 396: Write Stat of tid 52, mem usage: 577,556 KB, elapsed time: 28.707 s, ts: 1542091614.595 s
14:46:54 396: Write Stat of tid 49, mem usage: 577,808 KB, elapsed time: 28.707 s, ts: 1542091614.595 s
14:46:54 396: Write Stat of tid 55, mem usage: 578,060 KB, elapsed time: 28.707 s, ts: 1542091614.596 s
14:47:08 396: Write Stat of tid 42, mem usage: 580,068 KB, elapsed time: 43.079 s, ts: 1542091628.965 s
14:47:08 396: Write Stat of tid 45, mem usage: 580,068 KB, elapsed time: 43.079 s, ts: 1542091628.966 s
14:47:08 396: Write Stat of tid 43, mem usage: 580,108 KB, elapsed time: 43.080 s, ts: 1542091628.966 s
14:47:08 396: Write Stat of tid 41, mem usage: 580,108 KB, elapsed time: 43.081 s, ts: 1542091628.967 s
14:47:08 396: Write Stat of tid 44, mem usage: 580,108 KB, elapsed time: 43.080 s, ts: 1542091628.967 s
14:47:08 396: Write Stat of tid 40, mem usage: 580,108 KB, elapsed time: 43.082 s, ts: 1542091628.968 s
14:47:08 396: Write Stat of tid 47, mem usage: 580,108 KB, elapsed time: 43.082 s, ts: 1542091628.970 s
14:47:08 396: Write Stat of tid 46, mem usage: 580,108 KB, elapsed time: 43.088 s, ts: 1542091628.975 s
14:47:23 396: Write Stat of tid 33, mem usage: 578,084 KB, elapsed time: 57.386 s, ts: 1542091643.269 s
14:47:23 396: Write Stat of tid 32, mem usage: 578,336 KB, elapsed time: 57.387 s, ts: 1542091643.270 s
14:47:23 396: Write Stat of tid 34, mem usage: 578,588 KB, elapsed time: 57.387 s, ts: 1542091643.271 s
14:47:23 396: Write Stat of tid 38, mem usage: 578,840 KB, elapsed time: 57.388 s, ts: 1542091643.274 s
14:47:23 396: Write Stat of tid 35, mem usage: 579,092 KB, elapsed time: 57.390 s, ts: 1542091643.275 s
14:47:23 396: Write Stat of tid 36, mem usage: 579,336 KB, elapsed time: 57.390 s, ts: 1542091643.276 s
14:47:23 396: Write Stat of tid 37, mem usage: 579,588 KB, elapsed time: 57.394 s, ts: 1542091643.279 s
14:47:23 396: Write Stat of tid 39, mem usage: 579,840 KB, elapsed time: 57.395 s, ts: 1542091643.281 s
14:47:37 396: Write Stat of tid 27, mem usage: 578,068 KB, elapsed time: 71.688 s, ts: 1542091657.570 s
14:47:37 396: Write Stat of tid 30, mem usage: 578,320 KB, elapsed time: 71.688 s, ts: 1542091657.570 s
14:47:37 396: Write Stat of tid 28, mem usage: 578,572 KB, elapsed time: 71.689 s, ts: 1542091657.571 s
14:47:37 396: Write Stat of tid 25, mem usage: 578,824 KB, elapsed time: 71.690 s, ts: 1542091657.571 s
14:47:37 396: Write Stat of tid 26, mem usage: 579,076 KB, elapsed time: 71.690 s, ts: 1542091657.572 s
14:47:37 396: Write Stat of tid 29, mem usage: 579,328 KB, elapsed time: 71.691 s, ts: 1542091657.574 s
14:47:37 396: Write Stat of tid 24, mem usage: 579,580 KB, elapsed time: 71.693 s, ts: 1542091657.575 s
14:47:37 396: Write Stat of tid 31, mem usage: 579,832 KB, elapsed time: 71.694 s, ts: 1542091657.577 s
14:47:51 396: Write Stat of tid 18, mem usage: 580,084 KB, elapsed time: 85.986 s, ts: 1542091671.863 s
14:47:51 396: Write Stat of tid 17, mem usage: 580,336 KB, elapsed time: 85.988 s, ts: 1542091671.864 s
14:47:51 396: Write Stat of tid 16, mem usage: 580,588 KB, elapsed time: 85.989 s, ts: 1542091671.864 s
14:47:51 396: Write Stat of tid 22, mem usage: 580,840 KB, elapsed time: 85.985 s, ts: 1542091671.865 s
14:47:51 396: Write Stat of tid 20, mem usage: 581,092 KB, elapsed time: 85.986 s, ts: 1542091671.866 s
14:47:51 396: Write Stat of tid 21, mem usage: 581,344 KB, elapsed time: 85.986 s, ts: 1542091671.866 s
14:47:51 396: Write Stat of tid 19, mem usage: 581,596 KB, elapsed time: 85.989 s, ts: 1542091671.867 s
14:47:51 396: Write Stat of tid 23, mem usage: 581,848 KB, elapsed time: 85.988 s, ts: 1542091671.869 s
14:48:06 396: Write Stat of tid 14, mem usage: 582,100 KB, elapsed time: 100.284 s, ts: 1542091686.158 s
14:48:06 396: Write Stat of tid 15, mem usage: 582,352 KB, elapsed time: 100.286 s, ts: 1542091686.161 s
14:48:06 396: Write Stat of tid 12, mem usage: 582,604 KB, elapsed time: 100.291 s, ts: 1542091686.165 s
14:48:06 396: Write Stat of tid 13, mem usage: 582,856 KB, elapsed time: 100.291 s, ts: 1542091686.165 s
14:48:06 396: Write Stat of tid 10, mem usage: 582,856 KB, elapsed time: 100.296 s, ts: 1542091686.165 s
14:48:06 396: Write Stat of tid 11, mem usage: 583,140 KB, elapsed time: 100.297 s, ts: 1542091686.166 s
14:48:06 396: Write Stat of tid 9, mem usage: 583,180 KB, elapsed time: 100.298 s, ts: 1542091686.166 s
14:48:06 396: Write Stat of tid 8, mem usage: 583,220 KB, elapsed time: 100.299 s, ts: 1542091686.166 s
14:48:20 396: Write Stat of tid 7, mem usage: 581,236 KB, elapsed time: 114.592 s, ts: 1542091700.458 s
14:48:20 396: Write Stat of tid 6, mem usage: 581,276 KB, elapsed time: 114.592 s, ts: 1542091700.458 s
14:48:20 396: Write Stat of tid 0, mem usage: 581,316 KB, elapsed time: 114.617 s, ts: 1542091700.472 s
14:48:20 396: Write Stat of tid 4, mem usage: 581,356 KB, elapsed time: 114.608 s, ts: 1542091700.472 s
14:48:20 396: Write Stat of tid 1, mem usage: 581,396 KB, elapsed time: 114.616 s, ts: 1542091700.472 s
14:48:20 396: Write Stat of tid 3, mem usage: 581,428 KB, elapsed time: 114.612 s, ts: 1542091700.473 s
14:48:20 396: Write Stat of tid 2, mem usage: 581,468 KB, elapsed time: 114.617 s, ts: 1542091700.476 s
14:48:20 396: Write Stat of tid 5, mem usage: 581,508 KB, elapsed time: 114.614 s, ts: 1542091700.478 s
```