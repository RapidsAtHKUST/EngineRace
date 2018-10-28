package com.alibabacloud.polar_race.engine.common;

public class ByteUtils {
    static long bytesToLong(byte[] b) {
        long result = 0;
        for (int i = 0; i < 8; i++) {
            result <<= 8;
            result |= (b[i] & 0xFF);
        }
        return result;
    }

    static byte[] longToBytesInPlace(long l, byte[] result, int startOff) {
        for (int i = 7; i >= 0; i--) {
            result[startOff + i] = (byte) (l & 0xFF);
            l >>= 8;
        }
        return result;
    }

    static byte[] longToBytes(long l) {
        byte[] result = new byte[8];
        for (int i = 7; i >= 0; i--) {
            result[i] = (byte) (l & 0xFF);
            l >>= 8;
        }
        return result;
    }

    static long bytesToLongInPlace(byte[] b, int startOff) {
        long result = 0;
        for (int i = 0; i < 8; i++) {
            result <<= 8;
            result |= (b[i + startOff] & 0xFF);
        }
        return result;
    }
}
