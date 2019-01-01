### AIO Read Buckets

* `BS: {128}`, `QD:32`, `exp-get-events: 2` (2018-11-25_22-06-first-benchmark-range-96.3s-notbusywaiting)

```
22:01:41 INFO  (ts: 1543154501.710703) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:923: Total time: 99.119965 seconds.
22:03:18 INFO  (ts: 1543154598.093481) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1029: Total time: 96.369662 seconds.
```

* `BS: {64, 128, 256, 512, 1024}`, `QD:32`, `exp-get-events: 2`

```
22:17:42 INFO  (ts: 1543155462.012188) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:923: Total time: 99.244005 seconds.
22:19:18 INFO  (ts: 1543155558.510724) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1033: Total time: 96.485727 seconds.
22:20:55 INFO  (ts: 1543155655.026697) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1033: Total time: 96.503036 seconds.
22:22:31 INFO  (ts: 1543155751.540206) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1033: Total time: 96.500045 seconds.
22:24:08 INFO  (ts: 1543155848.067127) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1033: Total time: 96.512981 seconds.
22:25:44 INFO  (ts: 1543155944.599380) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1033: Total time: 96.518167 seconds.
```

* `BS: {8, 16, 32}`, `QD：32`, `exp-get-events: 2`


```
22:38:05 INFO  (ts: 1543156685.610966) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:923: Total time: 99.199276 seconds.
22:39:42 INFO  (ts: 1543156782.068386) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.444263 seconds.
22:41:18 INFO  (ts: 1543156878.545172) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.464142 seconds.
22:42:55 INFO  (ts: 1543156975.035088) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.477196 seconds.
```


* `BS: {8, 16, 32, 64, 128}`, `QD: 64`, `exp-get-events: 2`

```
22:51:21 INFO  (ts: 1543157481.751108) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:923: Total time: 99.170304 seconds.
22:52:58 INFO  (ts: 1543157578.192955) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.428920 seconds.
22:54:34 INFO  (ts: 1543157674.646562) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.440928 seconds.
22:56:11 INFO  (ts: 1543157771.100444) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.440891 seconds.
22:57:47 INFO  (ts: 1543157867.559667) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.445727 seconds.
22:59:24 INFO  (ts: 1543157964.025171) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.452168 seconds.
```


* `BS: {8, 16, 32, 64, 128}`, `QD: 64`, `exp-get-events: 0`

```
23:10:36 INFO  (ts: 1543158636.726747) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:923: Total time: 99.319473 seconds.
23:12:13 INFO  (ts: 1543158733.307876) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.567766 seconds.
23:13:49 INFO  (ts: 1543158829.899881) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.578484 seconds.
23:15:26 INFO  (ts: 1543158926.490623) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.577117 seconds.
23:17:03 INFO  (ts: 1543159023.087606) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.582820 seconds.
23:18:39 INFO  (ts: 1543159119.686401) /home/haiqing.shq/polardbrace/benchmark/engine/engine_race/engine_race.cc:1035: Total time: 96.584573 seconds.
```

### Others: Multi-Threading Read Different Buckets

* （187130, 2018-11-24_19-20)

```
99.046 s
19:09:29 795: Start TestRead(), time: 195.644 s, ts: 1543057769.770 s
19:09:29 413: In bucket 0, Read in tid: 2, start ts: 1543057769.770151138 s
19:09:29 436: In bucket 0, Read time 0.096395211 s, ts: 1543057769.866546392 s, tid: 2, acc-time: 0.096395 s
19:09:35 413: In bucket 64, Read in tid: 2, start ts: 1543057775.959104300 s
19:09:36 436: In bucket 64, Read time 0.096615328 s, ts: 1543057776.055719614 s, tid: 2, acc-time: 6.285528 s
19:09:42 413: In bucket 128, Read in tid: 2, start ts: 1543057782.149954557 s
19:09:42 436: In bucket 128, Read time 0.096625782 s, ts: 1543057782.246580124 s, tid: 2, acc-time: 12.476349 s
19:09:48 413: In bucket 192, Read in tid: 2, start ts: 1543057788.340886354 s
19:09:48 436: In bucket 192, Read time 0.096245640 s, ts: 1543057788.437131882 s, tid: 2, acc-time: 18.666859 s
19:09:54 413: In bucket 256, Read in tid: 2, start ts: 1543057794.530262709 s
19:09:54 436: In bucket 256, Read time 0.096589180 s, ts: 1543057794.626852036 s, tid: 2, acc-time: 24.856537 s
19:10:00 413: In bucket 320, Read in tid: 2, start ts: 1543057800.715174198 s
19:10:00 436: In bucket 320, Read time 0.096709965 s, ts: 1543057800.811883926 s, tid: 2, acc-time: 31.041528 s
19:10:06 413: In bucket 384, Read in tid: 2, start ts: 1543057806.907634974 s
19:10:07 436: In bucket 384, Read time 0.097466565 s, ts: 1543057807.005101681 s, tid: 2, acc-time: 37.234703 s
19:10:13 413: In bucket 448, Read in tid: 2, start ts: 1543057813.096128941 s
19:10:13 436: In bucket 448, Read time 0.096444461 s, ts: 1543057813.192573547 s, tid: 2, acc-time: 43.422134 s
19:10:19 413: In bucket 512, Read in tid: 2, start ts: 1543057819.285763502 s
19:10:19 436: In bucket 512, Read time 0.096880967 s, ts: 1543057819.382644653 s, tid: 2, acc-time: 49.612163 s
19:10:25 413: In bucket 576, Read in tid: 2, start ts: 1543057825.478718758 s
19:10:25 436: In bucket 576, Read time 0.097058851 s, ts: 1543057825.575777769 s, tid: 2, acc-time: 55.805251 s
19:10:31 413: In bucket 640, Read in tid: 2, start ts: 1543057831.664162874 s
19:10:31 436: In bucket 640, Read time 0.097284885 s, ts: 1543057831.761447668 s, tid: 2, acc-time: 61.990878 s
19:10:37 413: In bucket 704, Read in tid: 2, start ts: 1543057837.850811243 s
19:10:37 436: In bucket 704, Read time 0.096968947 s, ts: 1543057837.947779894 s, tid: 2, acc-time: 68.177163 s
19:10:44 413: In bucket 768, Read in tid: 2, start ts: 1543057844.045987844 s
19:10:44 436: In bucket 768, Read time 0.096392322 s, ts: 1543057844.142379999 s, tid: 2, acc-time: 74.371719 s
19:10:50 413: In bucket 832, Read in tid: 2, start ts: 1543057850.232035875 s
19:10:50 436: In bucket 832, Read time 0.096709879 s, ts: 1543057850.328745842 s, tid: 2, acc-time: 80.558044 s
19:10:56 413: In bucket 896, Read in tid: 2, start ts: 1543057856.420530081 s
19:10:56 436: In bucket 896, Read time 0.096242641 s, ts: 1543057856.516772747 s, tid: 2, acc-time: 86.746030 s
19:11:02 413: In bucket 960, Read in tid: 2, start ts: 1543057862.613217115 s
19:11:02 436: In bucket 960, Read time 0.097012234 s, ts: 1543057862.710229158 s, tid: 2, acc-time: 92.939440 s
19:11:08 803: End TestRead(), time: 99.046 s, ts: 1543057868.816 s

97.35700011253357s
19:11:08 814: Start TestM-2, time: 294.690 s, ts: 1543057868.816 s
19:11:08 826: End TestM-2, time: 0.000 s, ts: 1543057868.816 s
19:11:08 841: In bucket 0, Read in tid: 3, start ts: 1543057868.816259861 s
19:11:09 864: In bucket 0, Read time 0.191296023 s, ts: 1543057869.007555723 s, tid: 3, acc-time: 0.191296 s, buffer id: 3
19:11:14 841: In bucket 64, Read in tid: 3, start ts: 1543057874.812037945 s
19:11:15 864: In bucket 64, Read time 0.190398074 s, ts: 1543057875.002436161 s, tid: 3, acc-time: 6.186137 s, buffer id: 3
19:11:20 841: In bucket 128, Read in tid: 3, start ts: 1543057880.897819281 s
19:11:21 864: In bucket 128, Read time 0.190322641 s, ts: 1543057881.088141918 s, tid: 3, acc-time: 12.271808 s, buffer id: 3
19:11:27 841: In bucket 192, Read in tid: 3, start ts: 1543057887.017457247 s
19:11:27 864: In bucket 192, Read time 0.178647140 s, ts: 1543057887.196104527 s, tid: 3, acc-time: 18.379738 s, buffer id: 3
19:11:33 841: In bucket 256, Read in tid: 3, start ts: 1543057893.071447849 s
19:11:33 864: In bucket 256, Read time 0.189783877 s, ts: 1543057893.261231661 s, tid: 3, acc-time: 24.444836 s, buffer id: 3
19:11:39 841: In bucket 320, Read in tid: 3, start ts: 1543057899.210210800 s
19:11:39 864: In bucket 320, Read time 0.190340562 s, ts: 1543057899.400551319 s, tid: 3, acc-time: 30.584120 s, buffer id: 3
19:11:45 841: In bucket 384, Read in tid: 3, start ts: 1543057905.296273470 s
19:11:45 864: In bucket 384, Read time 0.202307330 s, ts: 1543057905.498580694 s, tid: 3, acc-time: 36.682121 s, buffer id: 3
19:11:51 841: In bucket 448, Read in tid: 3, start ts: 1543057911.373735428 s
19:11:51 864: In bucket 448, Read time 0.211697943 s, ts: 1543057911.585433245 s, tid: 3, acc-time: 42.768946 s, buffer id: 3
19:11:57 841: In bucket 512, Read in tid: 3, start ts: 1543057917.476566076 s
19:11:57 864: In bucket 512, Read time 0.176398543 s, ts: 1543057917.652964592 s, tid: 3, acc-time: 48.836449 s, buffer id: 3
19:12:03 841: In bucket 576, Read in tid: 3, start ts: 1543057923.568306208 s
19:12:03 864: In bucket 576, Read time 0.177239136 s, ts: 1543057923.745545149 s, tid: 3, acc-time: 54.928997 s, buffer id: 3
19:12:09 841: In bucket 640, Read in tid: 3, start ts: 1543057929.624368906 s
19:12:09 864: In bucket 640, Read time 0.195425881 s, ts: 1543057929.819794893 s, tid: 3, acc-time: 61.003213 s, buffer id: 3
19:12:15 841: In bucket 704, Read in tid: 3, start ts: 1543057935.649986267 s
19:12:15 864: In bucket 704, Read time 0.189901099 s, ts: 1543057935.839887619 s, tid: 3, acc-time: 67.023276 s, buffer id: 3
19:12:21 841: In bucket 768, Read in tid: 3, start ts: 1543057941.736797810 s
19:12:21 864: In bucket 768, Read time 0.190065362 s, ts: 1543057941.926863194 s, tid: 3, acc-time: 73.110216 s, buffer id: 3
19:12:27 841: In bucket 832, Read in tid: 3, start ts: 1543057947.816486120 s
19:12:28 864: In bucket 832, Read time 0.189159666 s, ts: 1543057948.005645752 s, tid: 3, acc-time: 79.188971 s, buffer id: 3
19:12:33 841: In bucket 896, Read in tid: 3, start ts: 1543057953.934641361 s
19:12:34 864: In bucket 896, Read time 0.206910738 s, ts: 1543057954.141552210 s, tid: 3, acc-time: 85.324847 s, buffer id: 3
19:12:39 841: In bucket 960, Read in tid: 3, start ts: 1543057959.984668732 s
19:12:40 864: In bucket 960, Read time 0.190030061 s, ts: 1543057960.174698830 s, tid: 3, acc-time: 91.357964 s, buffer id: 3

97.3109998703003s
19:12:46 814: Start TestM-3, time: 392.048 s, ts: 1543057966.173 s
19:12:46 841: 19:12:46 826: In bucket 0, Read in tid: 5, start ts: 1543057966.173964262 s
End TestM-3, time: 0.000 s, ts: 1543057966.173 s
19:12:46 864: In bucket 0, Read time 0.286030774 s, ts: 1543057966.459994793 s, tid: 5, acc-time: 0.286031 s, buffer id: 0
19:12:52 841: In bucket 64, Read in tid: 6, start ts: 1543057972.313080788 s
19:12:52 864: In bucket 64, Read time 0.284604006 s, ts: 1543057972.597684622 s, tid: 6, acc-time: 6.423706 s, buffer id: 1
19:12:58 841: In bucket 128, Read in tid: 7, start ts: 1543057978.036160946 s
19:12:58 864: In bucket 128, Read time 0.200770694 s, ts: 1543057978.236931801 s, tid: 7, acc-time: 12.062915 s, buffer id: 2
19:13:04 841: In bucket 192, Read in tid: 5, start ts: 1543057984.472856998 s
19:13:04 864: In bucket 192, Read time 0.286027792 s, ts: 1543057984.758884907 s, tid: 5, acc-time: 18.584865 s, buffer id: 0
19:13:10 841: In bucket 256, Read in tid: 6, start ts: 1543057990.495948553 s
19:13:10 864: In bucket 256, Read time 0.231351421 s, ts: 1543057990.727300167 s, tid: 6, acc-time: 24.553264 s, buffer id: 1
19:13:16 841: In bucket 320, Read in tid: 7, start ts: 1543057996.339209557 s
19:13:16 864: In bucket 320, Read time 0.314558918 s, ts: 1543057996.653768539 s, tid: 7, acc-time: 30.479697 s, buffer id: 2
19:13:22 841: In bucket 384, Read in tid: 5, start ts: 1543058002.583376169 s
19:13:22 864: In bucket 384, Read time 0.244833294 s, ts: 1543058002.828209400 s, tid: 5, acc-time: 36.654131 s, buffer id: 0
19:13:28 841: In bucket 448, Read in tid: 6, start ts: 1543058008.564748764 s
19:13:28 864: In bucket 448, Read time 0.327241520 s, ts: 1543058008.891990185 s, tid: 6, acc-time: 42.717899 s, buffer id: 1
19:13:34 841: In bucket 512, Read in tid: 7, start ts: 1543058014.712737322 s
19:13:35 864: In bucket 512, Read time 0.298308529 s, ts: 1543058015.011045694 s, tid: 7, acc-time: 48.836915 s, buffer id: 2
19:13:40 841: In bucket 576, Read in tid: 5, start ts: 1543058020.751731157 s
19:13:41 864: In bucket 576, Read time 0.271007730 s, ts: 1543058021.022738934 s, tid: 5, acc-time: 54.848608 s, buffer id: 0
19:13:46 841: In bucket 640, Read in tid: 6, start ts: 1543058026.915342808 s
19:13:47 864: In bucket 640, Read time 0.296670146 s, ts: 1543058027.212013006 s, tid: 6, acc-time: 61.037869 s, buffer id: 1
19:13:52 841: In bucket 704, Read in tid: 7, start ts: 1543058032.940303326 s
19:13:53 864: In bucket 704, Read time 0.284191303 s, ts: 1543058033.224494934 s, tid: 7, acc-time: 67.050307 s, buffer id: 2
19:13:58 841: In bucket 768, Read in tid: 5, start ts: 1543058038.890409708 s
19:13:59 864: In bucket 768, Read time 0.280752786 s, ts: 1543058039.171162367 s, tid: 5, acc-time: 72.996975 s, buffer id: 0
19:14:04 841: In bucket 832, Read in tid: 6, start ts: 1543058044.999733448 s
19:14:05 864: In bucket 832, Read time 0.305402428 s, ts: 1543058045.305135965 s, tid: 6, acc-time: 79.130936 s, buffer id: 1
19:14:11 841: In bucket 896, Read in tid: 7, start ts: 1543058051.649880409 s
19:14:11 864: In bucket 896, Read time 0.273862712 s, ts: 1543058051.923743010 s, tid: 7, acc-time: 85.749505 s, buffer id: 2
19:14:17 841: In bucket 960, Read in tid: 5, start ts: 1543058057.255563259 s
19:14:17 864: In bucket 960, Read time 0.301675612 s, ts: 1543058057.557239056 s, tid: 5, acc-time: 91.383003 s, buffer id: 0

97.4760000705719s
19:14:23 814: Start TestM-4, time: 489.358 s, ts: 1543058063.484 s
19:14:23 841: In bucket 0, Read in tid: 8, start ts: 1543058063.484777451 s
19:14:23 826: End TestM-4, time: 0.000 s, ts: 1543058063.484 s
19:14:23 864: In bucket 0, Read time 0.380112834 s, ts: 1543058063.864890337 s, tid: 8, acc-time: 0.380113 s, buffer id: 3
19:14:29 841: In bucket 64, Read in tid: 8, start ts: 1543058069.500177383 s
19:14:29 864: In bucket 64, Read time 0.379507371 s, ts: 1543058069.879684925 s, tid: 8, acc-time: 6.394874 s, buffer id: 3
19:14:35 841: In bucket 128, Read in tid: 8, start ts: 1543058075.511809111 s
19:14:35 864: In bucket 128, Read time 0.393683191 s, ts: 1543058075.905492306 s, tid: 8, acc-time: 12.420646 s, buffer id: 3
19:14:41 841: In bucket 192, Read in tid: 8, start ts: 1543058081.291141987 s
19:14:41 864: In bucket 192, Read time 0.406866072 s, ts: 1543058081.698007822 s, tid: 8, acc-time: 18.213129 s, buffer id: 3
19:14:47 841: In bucket 256, Read in tid: 8, start ts: 1543058087.519299030 s
19:14:47 864: In bucket 256, Read time 0.377450476 s, ts: 1543058087.896749496 s, tid: 8, acc-time: 24.411833 s, buffer id: 3
19:14:53 841: In bucket 320, Read in tid: 8, start ts: 1543058093.646083355 s
19:14:53 864: In bucket 320, Read time 0.340756085 s, ts: 1543058093.986839533 s, tid: 8, acc-time: 30.501885 s, buffer id: 3
19:14:59 841: In bucket 384, Read in tid: 8, start ts: 1543058099.660827160 s
19:15:00 864: In bucket 384, Read time 0.381214148 s, ts: 1543058100.042041063 s, tid: 8, acc-time: 36.557044 s, buffer id: 3
19:15:05 841: In bucket 448, Read in tid: 8, start ts: 1543058105.755777597 s
19:15:06 864: In bucket 448, Read time 0.386507991 s, ts: 1543058106.142285585 s, tid: 8, acc-time: 42.657242 s, buffer id: 3
19:15:11 841: In bucket 512, Read in tid: 8, start ts: 1543058111.960370541 s
19:15:12 864: In bucket 512, Read time 0.368194342 s, ts: 1543058112.328564644 s, tid: 8, acc-time: 48.843478 s, buffer id: 3
19:15:18 841: In bucket 576, Read in tid: 8, start ts: 1543058118.267813206 s
19:15:18 864: In bucket 576, Read time 0.360544402 s, ts: 1543058118.628357410 s, tid: 8, acc-time: 55.143240 s, buffer id: 3
19:15:24 841: In bucket 640, Read in tid: 8, start ts: 1543058124.320569277 s
19:15:24 864: In bucket 640, Read time 0.447647912 s, ts: 1543058124.768217325 s, tid: 8, acc-time: 61.283061 s, buffer id: 3
19:15:30 841: In bucket 704, Read in tid: 8, start ts: 1543058130.450345516 s
19:15:30 864: In bucket 704, Read time 0.370126086 s, ts: 1543058130.820471525 s, tid: 8, acc-time: 67.335275 s, buffer id: 3
19:15:36 841: In bucket 768, Read in tid: 8, start ts: 1543058136.559145689 s
19:15:36 864: In bucket 768, Read time 0.376506254 s, ts: 1543058136.935651779 s, tid: 8, acc-time: 73.450423 s, buffer id: 3
19:15:42 841: In bucket 832, Read in tid: 8, start ts: 1543058142.597060204 s
19:15:42 864: In bucket 832, Read time 0.379795180 s, ts: 1543058142.976855278 s, tid: 8, acc-time: 79.491587 s, buffer id: 3
19:15:48 841: In bucket 896, Read in tid: 8, start ts: 1543058148.648856878 s
19:15:49 864: In bucket 896, Read time 0.392207341 s, ts: 1543058149.041064262 s, tid: 8, acc-time: 85.555759 s, buffer id: 3
19:15:54 841: In bucket 960, Read in tid: 8, start ts: 1543058154.842937708 s
19:15:55 864: In bucket 960, Read time 0.396193468 s, ts: 1543058155.239130974 s, tid: 8, acc-time: 91.753785 s, buffer id: 3

97.48499989509583s
19:16:00 814: Start TestM-5, time: 586.834 s, ts: 1543058160.960 s
19:16:00 841: In bucket 0, Read in tid: 12, start ts: 1543058160.960749388 s
19:16:00 826: End TestM-5, time: 0.000 s, ts: 1543058160.960 s
19:16:01 864: In bucket 0, Read time 0.473437537 s, ts: 1543058161.434186697 s, tid: 12, acc-time: 0.473438 s, buffer id: 2
19:16:06 841: In bucket 64, Read in tid: 16, start ts: 1543058166.571273327 s
19:16:07 864: In bucket 64, Read time 0.495970604 s, ts: 1543058167.067243814 s, tid: 16, acc-time: 6.106424 s, buffer id: 1
19:16:12 841: In bucket 128, Read in tid: 15, start ts: 1543058172.605939627 s
19:16:12 864: In bucket 128, Read time 0.270459588 s, ts: 1543058172.876399279 s, tid: 15, acc-time: 11.915612 s, buffer id: 0
19:16:19 841: In bucket 192, Read in tid: 14, start ts: 1543058179.611309767 s
19:16:20 864: In bucket 192, Read time 0.482887939 s, ts: 1543058180.094197750 s, tid: 14, acc-time: 19.133415 s, buffer id: 4
19:16:25 841: In bucket 256, Read in tid: 13, start ts: 1543058185.640700579 s
19:16:26 864: In bucket 256, Read time 0.476840006 s, ts: 1543058186.117540598 s, tid: 13, acc-time: 25.156763 s, buffer id: 3
19:16:31 841: In bucket 320, Read in tid: 12, start ts: 1543058191.047338247 s
19:16:31 864: In bucket 320, Read time 0.477219458 s, ts: 1543058191.524557829 s, tid: 12, acc-time: 30.563741 s, buffer id: 2
19:16:36 841: In bucket 384, Read in tid: 16, start ts: 1543058196.955400705 s
19:16:37 864: In bucket 384, Read time 0.476250629 s, ts: 1543058197.431651354 s, tid: 16, acc-time: 36.470763 s, buffer id: 1
19:16:43 841: In bucket 448, Read in tid: 15, start ts: 1543058203.063215256 s
19:16:43 864: In bucket 448, Read time 0.448096401 s, ts: 1543058203.511311769 s, tid: 15, acc-time: 42.550470 s, buffer id: 0
19:16:50 841: In bucket 512, Read in tid: 14, start ts: 1543058210.001990318 s
19:16:50 864: In bucket 512, Read time 0.482648634 s, ts: 1543058210.484638929 s, tid: 14, acc-time: 49.523799 s, buffer id: 4
19:16:56 841: In bucket 576, Read in tid: 13, start ts: 1543058216.100038767 s
19:16:56 864: In bucket 576, Read time 0.459397732 s, ts: 1543058216.559436321 s, tid: 13, acc-time: 55.598603 s, buffer id: 3
19:17:01 841: In bucket 640, Read in tid: 12, start ts: 1543058221.504863262 s
19:17:01 864: In bucket 640, Read time 0.456491171 s, ts: 1543058221.961354256 s, tid: 12, acc-time: 61.000484 s, buffer id: 2
19:17:07 841: In bucket 704, Read in tid: 16, start ts: 1543058227.403427362 s
19:17:07 864: In bucket 704, Read time 0.478548167 s, ts: 1543058227.881975651 s, tid: 16, acc-time: 66.921027 s, buffer id: 1
19:17:13 841: In bucket 768, Read in tid: 15, start ts: 1543058233.497040987 s
19:17:13 864: In bucket 768, Read time 0.485808438 s, ts: 1543058233.982849360 s, tid: 15, acc-time: 73.021953 s, buffer id: 0
19:17:20 841: In bucket 832, Read in tid: 14, start ts: 1543058240.460263968 s
19:17:20 864: In bucket 832, Read time 0.534154388 s, ts: 1543058240.994418144 s, tid: 14, acc-time: 80.033526 s, buffer id: 4
19:17:26 841: In bucket 896, Read in tid: 13, start ts: 1543058246.601908684 s
19:17:27 864: In bucket 896, Read time 0.471207659 s, ts: 1543058247.073116302 s, tid: 13, acc-time: 86.112230 s, buffer id: 3
19:17:31 841: In bucket 960, Read in tid: 12, start ts: 1543058251.835741520 s
19:17:32 864: In bucket 960, Read time 0.481567608 s, ts: 1543058252.317308903 s, tid: 12, acc-time: 91.356371 s, buffer id: 2
19:17:38 249: Start ~EngineRace(), time: 684.319 s, ts: 1543058258.445 s
```
