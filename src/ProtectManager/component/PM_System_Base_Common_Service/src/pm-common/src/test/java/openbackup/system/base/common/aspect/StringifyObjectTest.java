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
package openbackup.system.base.common.aspect;

import openbackup.system.base.common.aspect.StringifyConverter.StringifyObject;
import openbackup.system.base.common.utils.StringUtil;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * Stringify Converter Test
 *
 */
public class StringifyObjectTest {
    @Test
    public void test_convert() {
        Assert.assertEquals("1", new StringifyObject(new DataItem("1")).get("value").toString());
        List<?> items = Arrays.asList(new DataItem("1"), new DataItem("2"));
        Assert.assertEquals("1 2", new StringifyObject(items).get("value").toString());
        Assert.assertEquals("1 2", new StringifyObject(items).get("value").get("toString").toString());
        Assert.assertEquals("1", StringUtil.stringify(new StringifyObject(new DataItem("1")).get("value")));
    }



    private static class DataItem {
        private final String data;

        public DataItem(String data) {
            this.data = data;
        }

        public String value() {
            return data;
        }
    }
}
