/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.common.utils.security.Base64;

import org.junit.Assert;
import org.junit.Test;

public class Base64Test {
    @Test
    public void testByteArrayToBase64() {
        byte[] bytes1 = new byte[] {'d', 'f', 'a', 'b'};
        Assert.assertEquals(Base64.byteArrayToBase64(bytes1), "ZGZhYg==");
        byte[] bytes2 = new byte[] {'d', 'f', 'a', 'b', 'c', '9', '8', '7'};
        Assert.assertEquals(Base64.byteArrayToBase64(bytes2), "ZGZhYmM5ODc=");
    }

    @Test
    public void testBase64ToByteArray() {
        byte[] bytes1 = new byte[] {'d', 'f', 'a', 'b'};
        Assert.assertArrayEquals(Base64.base64ToByteArray("ZGZhYg=="), bytes1);
        byte[] bytes2 = new byte[] {'d', 'f', 'a', 'b', 'c', '9', '8', '7'};
        Assert.assertArrayEquals(Base64.base64ToByteArray("ZGZhYmM5ODc="), bytes2);
    }
}
