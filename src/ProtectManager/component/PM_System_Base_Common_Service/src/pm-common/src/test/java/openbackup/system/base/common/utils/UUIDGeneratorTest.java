/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.common.utils.UUIDGenerator;

import org.junit.Assert;
import org.junit.Test;

/**
 * UUIDGeneratorTest
 *
 * @author w00439064
 * @since 2021-04-05
 */
public class UUIDGeneratorTest {
    /**
     * testGetUUID
     */
    @Test
    public void testGetUUID() {
        String[] arr = UUIDGenerator.getUUID(2);
        Assert.assertFalse(arr[0].contains("-"));
        Assert.assertEquals(arr.length, 2);
    }
}
