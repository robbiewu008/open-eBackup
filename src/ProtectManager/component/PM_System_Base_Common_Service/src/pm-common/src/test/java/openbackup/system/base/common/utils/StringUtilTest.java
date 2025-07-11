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

import openbackup.system.base.common.utils.StringUtil;

import org.junit.Assert;
import org.junit.Test;

import java.io.UnsupportedEncodingException;

/**
 * StringUtilTest
 *
 */
public class StringUtilTest {
    /**
     * testArrayToString
     */
    @Test
    public void testArrayToString() {
        Assert.assertEquals(StringUtil.arrayToString(new String[] {"A", "B"}), "A:B");
        Assert.assertEquals(StringUtil.arrayToString(null), "");
        Assert.assertEquals(StringUtil.arrayToString(new String[] {}), "");
    }

    /**
     * testSubstr
     */
    @Test
    public void testSubstr() throws UnsupportedEncodingException {
        Assert.assertNull(StringUtil.substr(null, 1, ""));
        Assert.assertEquals(StringUtil.substr("123", 1, ""), "1");
        Assert.assertEquals(StringUtil.substr("abc", 2, "utf-8"), "ab");
    }

    @Test
    public void testCleanString001() {
        String string = null;
        StringUtil.clean(string);
        Assert.assertEquals(null, string);
    }

    @Test
    public void testCleanString002() {
        String string = "";
        StringUtil.clean(string);
        Assert.assertEquals("", string);
    }

    @Test
    public void testCleanString003() {
        String string = "a";
        StringUtil.clean(string);
        Assert.assertEquals(1, string.length());
        char[] chars = string.toCharArray();
        for (int i = 0; i < chars.length; i++) {
            Assert.assertEquals(0, chars[i]);
        }
    }

    @Test
    public void testCleanString004() {
        String string = "xxxxxxxx";
        StringUtil.clean(string);
        Assert.assertEquals(8, string.length());
        char[] chars = string.toCharArray();
        for (int i = 0; i < chars.length; i++) {
            Assert.assertEquals(0, chars[i]);
        }
    }

    @Test
    public void testCleanChars001() {
        char[] chars = null;
        StringUtil.clean(chars);
        Assert.assertEquals(null, chars);
    }

    @Test
    public void testCleanChars002() {
        char[] chars = new char[] {};
        StringUtil.clean(chars);
        Assert.assertEquals(chars, chars);
    }

    @Test
    public void testCleanChars003() {
        int size = 5;
        char[] chars = new char[size];
        StringUtil.clean(chars);
        Assert.assertEquals(size, chars.length);
    }

    @Test
    public void testCleanChars004() {
        int size = 1024;
        char[] chars = new char[size];
        StringUtil.clean(chars);
        Assert.assertEquals(size, chars.length);
        for (int i = 0; i < size; i++) {
            Assert.assertEquals(0, chars[i]);
        }
    }
}
