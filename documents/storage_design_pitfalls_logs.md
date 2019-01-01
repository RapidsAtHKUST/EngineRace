## Single-File, Single-AIO-Writer (16KB Value Write Buffer)

### No-Fallocate

* 2018-12-06_10-55-test-aio-single-file-nofallocate-write467s-read375s-readseq-ok-QD0.5w

### Fallocate

* 2018-12-06_11-40-write263s-read362s-avgqs=wirte1.3-read1.5

## 64 Files

### Single-Writer

* 2018-12-06_12-50-64file-write240s-readok
* 2018-12-06_13-10-write128KB-116.9s-readok
* 2018-12-06_13-30-write1MB-258s
    * seem to be split to 24or more io requests per pwrite
* 2018-12-06_13-50-write64KB-116.2s

## 8 Writers

* 2018-12-06_14-51-8write-threads-QD=16perwriter-fallocate
    * write: 115.78s
* 2018-12-06_15-05-QD=8-disable-fallocate
    * write: 116.5s

## 16 Writers

* 2018-12-06_16-00-16threadswriter-fallocate-QD=8

## A normal One For Comparison

* 2018-12-06_17-15-normal

sample write:

```
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
sda               0.00     4.00    0.00    2.00     0.00     0.02    24.00     0.00    0.00    0.00    0.00   0.00   0.00
nvme0n1           0.00     0.00    0.00 140404.00     0.00  2181.39    31.82    23.23    0.16    0.00    0.16   0.01 100.80
```