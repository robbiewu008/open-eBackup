/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.common.utils.network;

import openbackup.system.base.common.utils.network.BitSetHelpers;

import org.junit.Assert;
import org.junit.Test;

import java.util.BitSet;

public class BitSetHelpersTest {
    @Test
    public void testBitSetOf() {
        BitSet bitSet = new BitSet(2);
        bitSet.set(0);
        bitSet.set(64);
        bitSet.set(65);
        BitSet set = BitSetHelpers.bitSetOf(1L, 3L);
        Assert.assertEquals(bitSet.get(0, 2), set.get(0, 2));
    }
}
