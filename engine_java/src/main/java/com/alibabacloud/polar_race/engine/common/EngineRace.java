package com.alibabacloud.polar_race.engine.common;

import com.alibabacloud.polar_race.engine.common.exceptions.EngineException;
import gnu.trove.map.hash.TLongIntHashMap;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.StandardOpenOption;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReentrantLock;

public class EngineRace extends AbstractEngine {
    private static int PARTITION_SIZE = 128;
    private static int KEY_SIZE = 8;
    private static int INDEX_ENTRY_SIZE = 12;
    private static int VALUE_SIZE = 4096;
    private static long VALUE_REDIS_SIZE = 64L * 1024 * 1024 * VALUE_SIZE;      // 256GB exactly
    private static long INDEX_META_REDIS_SIZE = 8 * PARTITION_SIZE;     // count size
    private static long INDEX_REDIS_SIZE = INDEX_ENTRY_SIZE * 1024L * 1024 * 64 / PARTITION_SIZE;

    private static ThreadLocal<Integer> integerThreadLocalRead = new ThreadLocal<>();
    private static ThreadLocal<ByteBuffer> valueBufferThreadLocal = new ThreadLocal<>();
    private static ThreadLocal<ByteBuffer> valueReadBufferThreadLocal = new ThreadLocal<>();

    // 1st: in-memory index-related structures
    private TLongIntHashMap[] hashMapArr = new TLongIntHashMap[PARTITION_SIZE];
    private int[] cntArr = new int[PARTITION_SIZE];
    private ReentrantLock[] reentrantLocks = new ReentrantLock[PARTITION_SIZE];
    // 1st-continuing: out-of-core index-related structures
    private MappedByteBuffer cntMappedByteBufArr;
    private FileChannel[] indexChannelArr = new FileChannel[PARTITION_SIZE];

    // 2nd: out-of-core data
    private static AtomicInteger valueIdx = new AtomicInteger();
    private FileChannel valueChannel;

    @Override
    public void open(String path) throws EngineException {
        // 1st: restore the index meta count info
        int cnt = -1;
        File indexMetaFile = new File(path + File.separator + "index-meta" + ".redis");
        FileChannel indexMetaChannel;
        try {
            boolean is_first = false;
            if (!indexMetaFile.exists()) {
                is_first = true;
                indexMetaChannel = FileChannel.open(indexMetaFile.toPath(),
                        StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.CREATE);
                indexMetaChannel.truncate(INDEX_META_REDIS_SIZE);
            } else {
                indexMetaChannel = FileChannel.open(indexMetaFile.toPath(),
                        StandardOpenOption.READ, StandardOpenOption.WRITE);
            }
            cntMappedByteBufArr = indexMetaChannel.map(FileChannel.MapMode.READ_WRITE,
                    0, INDEX_META_REDIS_SIZE);
            if (is_first) {
                for (int i = 0; i < PARTITION_SIZE; i++) {
                    cntMappedByteBufArr.putInt(4 * i, 0);
                }
            }
            for (int i = 0; i < PARTITION_SIZE; i++) {
                cntArr[i] = cntMappedByteBufArr.getInt(4 * i);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        // 2nd: restore the index info
        for (int i = 0; i < PARTITION_SIZE; i++) {
            // init hash tables and locks
            hashMapArr[i] = new TLongIntHashMap(10000);
            hashMapArr[i].setAutoCompactionFactor(0.9f);
            reentrantLocks[i] = new ReentrantLock();
            File indexFile = new File(path + File.separator + "index-" + i + ".redis");
            try {
                if (indexFile.exists()) {
                    DataInputStream indexStream = new DataInputStream(
                            new BufferedInputStream(new FileInputStream(indexFile)));
                    for (int j = 0; j < cntArr[i]; j++) {
                        long key = indexStream.readLong();
                        int value = indexStream.readInt();
                        hashMapArr[i].put(key, value);
                    }
                    cnt += cntArr[i];
                    indexStream.close();
                    indexChannelArr[i] = FileChannel.open(indexFile.toPath(),
                            StandardOpenOption.READ, StandardOpenOption.WRITE);
                } else {
                    indexChannelArr[i] = FileChannel.open(indexFile.toPath(),
                            StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.CREATE);
                    indexChannelArr[i].truncate(INDEX_REDIS_SIZE);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        valueIdx.set(cnt);
        System.out.println("cnt:" + cnt);

        // 3rd: init the data file, i.e., value channel
        String valueFileName = "value.redis";
        File dataFile = new File(path + File.separator + valueFileName);
        try {
            if (!dataFile.exists()) {
                valueChannel = FileChannel.open(dataFile.toPath(),
                        StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.CREATE);
                valueChannel.truncate(VALUE_REDIS_SIZE);
            } else {
                valueChannel = FileChannel.open(dataFile.toPath(),
                        StandardOpenOption.READ, StandardOpenOption.WRITE);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println(path);
    }

    @Override
    public void write(byte[] key, byte[] value) throws EngineException {
        long keyInt = ByteUtils.bytesToLong(key);
        long valueId = valueIdx.incrementAndGet();

        if (valueBufferThreadLocal.get() == null) {
            valueBufferThreadLocal.set(ByteBuffer.allocate(VALUE_SIZE));
        }
        ByteBuffer buffer = valueBufferThreadLocal.get();

        // 1st: update and write the index
        buffer.clear();
        int partitionId = (int) (keyInt % PARTITION_SIZE);

        reentrantLocks[partitionId].lock();
        hashMapArr[partitionId].put(keyInt, (int) valueId);
        int myCnt = ++cntArr[partitionId];
        if (myCnt % 5000 == 0) {
            System.out.println(myCnt + ", tid:" + Thread.currentThread().getId());
        }
        cntMappedByteBufArr.putInt(4 * partitionId, myCnt);
        reentrantLocks[partitionId].unlock();

        buffer.putLong(keyInt);
        buffer.putInt((int) valueId);
        buffer.flip();
        try {
            indexChannelArr[partitionId].write(buffer, (myCnt - 1) * INDEX_ENTRY_SIZE);
        } catch (IOException e) {
            e.printStackTrace();
        }

        // 2nd: write the value
        buffer.clear();
        buffer.put(value);
        buffer.flip();
        long offset = valueId * VALUE_SIZE;
        try {
            valueChannel.write(buffer, offset);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public byte[] read(byte[] key) throws EngineException {
        if (integerThreadLocalRead.get() == null) {
            integerThreadLocalRead.set(1);
            System.out.println("first read");
        }
        // 1st: lookup index
        long keyInt = ByteUtils.bytesToLong(key);
        int partitionId = (int) (keyInt % PARTITION_SIZE);
        long valueId = hashMapArr[partitionId].get(keyInt);
//        System.out.println("key:" + keyInt + ", valueId:" + valueId);
        long offset = valueId * VALUE_SIZE;

        // 2nd: read value
        if (valueReadBufferThreadLocal.get() == null) {
            valueReadBufferThreadLocal.set(ByteBuffer.allocate(VALUE_SIZE));
        }
        ByteBuffer buffer = valueReadBufferThreadLocal.get();
        buffer.clear();
        try {
            valueChannel.read(buffer, offset);
        } catch (IOException e) {
            e.printStackTrace();
        }
        byte[] value = new byte[VALUE_SIZE];
        System.arraycopy(buffer.array(), 0, value, 0, VALUE_SIZE);
        return value;
    }

    @Override
    public void range(byte[] lower, byte[] upper, AbstractVisitor visitor) throws EngineException {
    }

    @Override
    public void close() {
        System.out.println("close hook, tid:" + Thread.currentThread().getId());
    }
}
