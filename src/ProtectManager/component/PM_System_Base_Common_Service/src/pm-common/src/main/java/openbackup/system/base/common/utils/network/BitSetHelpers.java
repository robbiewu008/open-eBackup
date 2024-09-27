/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.network;

import java.util.BitSet;

/**
 * This class contains some helpers for working with BitSets.
 * These are generally not necessary in JDK7, since the BitSet.valueOf(long[])
 * method. However, for java-6 compatibility, we go this way.
 *
 * @author y00413474
 * @since 2020-07-01
 */
class BitSetHelpers {
    static BitSet bitSetOf(long lowerBits, long upperBits) {
        final BitSet bitSet = new BitSet();
        convert(lowerBits, 0, bitSet);
        convert(upperBits, Long.SIZE, bitSet);
        return bitSet;
    }

    static void convert(long value, int bitSetOffset, BitSet bits) {
        long input = value;
        int index = 0;
        while (input != 0L) {
            if (input % 2L != 0) {
                bits.set(bitSetOffset + index);
            }
            ++index;
            input = input >>> 1;
        }
    }
}
