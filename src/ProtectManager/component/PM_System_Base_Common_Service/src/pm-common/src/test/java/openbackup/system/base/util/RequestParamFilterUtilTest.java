/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.util.RequestParamFilterUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * RequestParamFilterUtilTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-26
 */
public class RequestParamFilterUtilTest {
    /**
     * 用例场景：通配符处理
     * 前置条件：服务正常，输入一个含有\的字符串
     * 检查点：返回一个预期字符串
     */
    @Test
    public void escape_value_success_when_value_contain_backslash() {
        String test = RequestParamFilterUtil.escape("\\\\test");
        Assert.assertEquals("\\\\test", test);
    }

    /**
     * 用例场景：通配符处理
     * 前置条件：服务正常，输入一个含有%的字符串
     * 检查点：返回一个预期字符串
     */
    @Test
    public void escape_value_success_when_value_contain_percent_sign() {
        String test = RequestParamFilterUtil.escape("%test");
        Assert.assertEquals("\\%test", test);
    }

    /**
     * 用例场景：通配符处理
     * 前置条件：服务正常，输入一个含有?的字符串
     * 检查点：返回一个预期字符串
     */
    @Test
    public void escape_value_success_when_value_contain_question_mark() {
        String test = RequestParamFilterUtil.escape("\\?test");
        Assert.assertEquals("\\\\?test", test);
    }

    /**
     * 用例场景：通配符处理
     * 前置条件：服务正常，输入一个含有_的字符串
     * 检查点：返回一个预期字符串
     */
    @Test
    public void escape_value_success_when_value_contain_low_line() {
        String test = RequestParamFilterUtil.escape("\\_test");
        Assert.assertEquals("\\\\_test", test);
    }

    /**
     * 用例场景：通配符处理
     * 前置条件：服务正常，输入null
     * 检查点：返回null
     */
    @Test
    public void escape_value_success_and_return_null_when_value_is_null() {
        String test = RequestParamFilterUtil.escape(null);
        Assert.assertNull(test);
    }
}
