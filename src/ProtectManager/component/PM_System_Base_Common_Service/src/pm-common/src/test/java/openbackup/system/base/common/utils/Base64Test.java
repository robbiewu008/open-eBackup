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
