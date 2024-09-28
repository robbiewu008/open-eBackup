/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.common.utils.network;

import java.util.BitSet;

/**
 * This class contains some helpers for working with BitSets.
 * These are generally not necessary in JDK7, since the BitSet.valueOf(long[])
 * method. However, for java-6 compatibility, we go this way.
 *
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
