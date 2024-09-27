/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.util.ListStringChecker;
import openbackup.system.base.util.TypeMode;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述
 *
 * @author w00426202
 * @since 2023-05-18
 */
public class ListStringCheckerTest {
    /**
     * 用例名称：测试集合大小
     * 前置条件：集合大小为5， 注解设置集合大小为2 <br/>
     * check点：校验失败<br/>
     */
    @Test
    public void testCollectionSizeExceed() {
        ListStringChecker listStringChecker = new ListStringChecker();

        listStringChecker.setMinSize(1);
        listStringChecker.setMaxSize(2);
        listStringChecker.setTotalLength(100);
        List<String> strTmp = Arrays.asList("hello", "hi", "萨瓦迪卡", "你好","阿尼哈撒哟");

        Assert.assertFalse(listStringChecker.isValid(strTmp, null));
    }

    /**
     * 用例名称：校验集合中的每个字符串长度
     * 前置条件：集合中有字符串长度超过5, 注解设置集合中的字符串长度为2<br/>
     * check点：校验失败<br/>
     */
    @Test
    public void testStrSizeExceed() {
        ListStringChecker listStringChecker = new ListStringChecker();

        listStringChecker.setMinSize(1);
        listStringChecker.setMaxSize(2);
        listStringChecker.setStrMaxLength(2);
        listStringChecker.setTotalLength(100);

        // 中文一个汉字等于两个字符
        List<String> strTmp = Arrays.asList("he", "hi","强者");

        Assert.assertFalse(listStringChecker.isValid(strTmp, null));
    }

    /**
     * 用例名称：校验集合中的每个字符串总长度
     * 前置条件：集合字符串总长度4<br/>
     * check点：校验失败<br/>
     */
    @Test
    public void testTotalStrLength() {
        ListStringChecker listStringChecker = new ListStringChecker();

        listStringChecker.setMinSize(1);
        listStringChecker.setMaxSize(100);
        listStringChecker.setStrMaxLength(10);
        listStringChecker.setTotalLength(4);

        // 中文一个汉字等于两个字符
        List<String> strTmp = Arrays.asList("he", "hi","you");

        Assert.assertFalse(listStringChecker.isValid(strTmp, null));
    }

    /**
     * 用例名称：测试特殊字符
     * 前置条件：注解指明特殊字符,字符串包含特殊字符<br/>
     * check点：校验失败<br/>
     */
    @Test
    public void testSpecialKey() {
        ListStringChecker listStringChecker = new ListStringChecker();

        listStringChecker.setMinSize(1);
        listStringChecker.setMaxSize(10);
        listStringChecker.setStrMaxLength(10);
        listStringChecker.setTotalLength(100);
        listStringChecker.setStrRegexp("");
        listStringChecker.setSpecialChars("");
        listStringChecker.setShouldSpecialCharCheck(true);
        listStringChecker.setSpecialKey(TypeMode.COMMON);

        // 中文一个汉字等于两个字符
        List<String> strTmp = Arrays.asList("he", "hi","you","hello*123","#");

        Assert.assertFalse(listStringChecker.isValid(strTmp, null));
    }

    /**
     * 用例名称：测试字符正则
     * 前置条件：注解设置正则，测试字符串的内容不符合正则<br/>
     * check点：校验失败<br/>
     */
    @Test
    public void testStrReg() {
        ListStringChecker listStringChecker = new ListStringChecker();

        listStringChecker.setMinSize(1);
        listStringChecker.setMaxSize(10);
        listStringChecker.setStrMaxLength(10);
        listStringChecker.setTotalLength(100);
        listStringChecker.setStrRegexp("");
        listStringChecker.setSpecialChars("");
        listStringChecker.setStrRegexp(RegexpConstants.NAME_STR);

        // 中文一个汉字等于两个字符
        List<String> strTmp = Arrays.asList("he", "hi","you","hello*123");
        Assert.assertFalse(listStringChecker.isValid(strTmp, null));
    }
}
