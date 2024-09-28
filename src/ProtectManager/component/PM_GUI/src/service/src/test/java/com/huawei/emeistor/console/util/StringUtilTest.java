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
package com.huawei.emeistor.console.util;

import org.junit.Assert;
import org.junit.Test;

public class StringUtilTest {

    /**
     * 用例场景：测试清理工具
     * 前置条件：清理成功
     * 检查点：清理成功
     */
    @Test
    public void test_clean_string_001() {
        String string = null;
        StringUtil.tripleWriteZero(string);
        Assert.assertEquals(null, string);
    }

    /**
     * 用例场景：测试清理工具
     * 前置条件：清理成功
     * 检查点：清理成功
     */
    @Test
    public void test_clean_string_002() {
        String string = "";
        StringUtil.tripleWriteZero(string);
        Assert.assertEquals("", string);
    }

    /**
     * 用例场景：测试清理工具
     * 前置条件：清理成功
     * 检查点：清理成功
     */
    @Test
    public void test_clean_string_003() {
        String string = "a";
        StringUtil.tripleWriteZero(string);
        Assert.assertEquals(1, string.length());
        char[] chars = string.toCharArray();
        for (int i = 0; i < chars.length; i++) {
            Assert.assertEquals(0, chars[i]);
        }
    }

    /**
     * 用例场景：测试清理工具
     * 前置条件：清理成功
     * 检查点：清理成功
     */
    @Test
    public void test_clean_string_004() {
        String string = new String("xxxxxxxx");
        StringUtil.tripleWriteZero(string);
        Assert.assertEquals(8, string.length());
        char[] chars = string.toCharArray();
        for (int i = 0; i < chars.length; i++) {
            Assert.assertEquals(0, chars[i]);
        }
    }
}
