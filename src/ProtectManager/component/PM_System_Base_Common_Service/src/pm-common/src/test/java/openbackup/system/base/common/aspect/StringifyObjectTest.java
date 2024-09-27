/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author l00272247
 * @since 2021-02-26
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
