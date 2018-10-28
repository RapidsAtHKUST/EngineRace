package com.alibabacloud.polar_race.engine.common;


import com.alibabacloud.polar_race.engine.common.exceptions.EngineException;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class EngineRacePlayground {
    private static int NUM_THREADS = 64;
    private static int SEED = 10;
    private static ExecutorService executorService;
    private static int KEY_SIZE = 8;
    private static int VALUE_SIZE = 4096;

    public static class WriteTask implements Runnable {
        EngineRace engineRace;
        long start;
        long end;

        WriteTask(EngineRace engineRace, long start, long end) {
            this.engineRace = engineRace;
            this.start = start;
            this.end = end;
        }

        @Override
        public void run() {
            for (long i = this.start; i < this.end; i++) {
                byte[] key = ByteUtils.longToBytes(i);
                byte[] value = new byte[VALUE_SIZE];
                for (int j = 0; j < VALUE_SIZE; j += KEY_SIZE) {
                    ByteUtils.longToBytesInPlace(j + i + SEED, value, j);
                }
                try {
                    engineRace.write(key, value);
                } catch (EngineException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static class ReadTask implements Runnable {
        EngineRace engineRace;
        long start;
        long end;

        ReadTask(EngineRace engineRace, long start, long end) {
            this.engineRace = engineRace;
            this.start = start;
            this.end = end;
        }

        @Override
        public void run() {
            for (long i = this.start; i < this.end; i++) {
                byte[] key = ByteUtils.longToBytes(i);
                try {
                    byte[] value = engineRace.read(key);
                    for (int j = 0; j < VALUE_SIZE; j += KEY_SIZE) {
                        if (ByteUtils.bytesToLongInPlace(value, j) != i + j + SEED) {
                            System.out.println("err");
                            System.out.println(ByteUtils.bytesToLongInPlace(value, j) + " " + (i + j + SEED));
                            System.exit(-1);
                        }
                    }
                } catch (EngineException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private static void Write(long iStart, long iEnd) throws EngineException {
        System.out.println("beg " + iStart + " , " + iEnd);
        executorService = Executors.newFixedThreadPool(NUM_THREADS);
        EngineRace engineRace = new EngineRace();
        engineRace.open("/tmp/test_engine_java");
        long avg = (iEnd - iStart) / NUM_THREADS + 1;
        for (long i = 0; i < iEnd; i += avg) {
            executorService.submit(new WriteTask(engineRace, i, Math.min(iEnd, i + avg)));
        }
        executorService.shutdown();
        try {
            executorService.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        engineRace.close();
        System.out.println("end " + iStart + " , " + iEnd);
    }

    private static void Read(long iStart, long iEnd) throws EngineException {
        executorService = Executors.newFixedThreadPool(NUM_THREADS);
        EngineRace engineRace = new EngineRace();
        engineRace.open("/tmp/test_engine_java");

        long avg = (iEnd - iStart) / NUM_THREADS + 1;
        for (long i = 0; i < iEnd; i += avg) {
            executorService.submit(new ReadTask(engineRace, i, Math.min(iEnd, i + avg)));
        }
        executorService.shutdown();
        try {
            executorService.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) throws EngineException {
        // 1st: write
        long ITER_NUM = 3;
        long EACH_ITER_SIZE = 200000;
        for (long iter = 0; iter < ITER_NUM; iter++)
            Write(iter * EACH_ITER_SIZE, (iter + 1) * EACH_ITER_SIZE);
        // 2nd: read
        Read(0, ITER_NUM * EACH_ITER_SIZE);
    }
}
