package com.alibabacloud.polar_race.engine.common;

import com.alibabacloud.polar_race.engine.common.exceptions.EngineException;

public class EngineRace extends AbstractEngine {
    static ThreadLocal<Integer> integerThreadLocal = new ThreadLocal<Integer>();
    static ThreadLocal<Integer> integerThreadLocalRead = new ThreadLocal<Integer>();

    @Override
    public void open(String path) throws EngineException {
        System.out.println(path);
    }

    @Override
    public void write(byte[] key, byte[] value) throws EngineException {
        if (integerThreadLocal.get() == null) {
            integerThreadLocal.set(0);
        }
        int curVal = integerThreadLocal.get();
        if (curVal % 100000 == 0) {
            System.out.println(Thread.currentThread().getId() + ", " + curVal);
        }
        integerThreadLocal.set(curVal + 1);
    }

    @Override
    public byte[] read(byte[] key) throws EngineException {
        byte[] value = null;

        return value;
    }

    @Override
    public void range(byte[] lower, byte[] upper, AbstractVisitor visitor) throws EngineException {
    }

    @Override
    public void close() {
        System.out.println(Thread.currentThread().getId());
    }

}
