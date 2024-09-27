/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.util.InputStringChecker;
import openbackup.system.base.util.TypeMode;

import org.junit.Assert;
import org.junit.Test;

/**
 * InputStringChecker测试类
 *
 * @author w00426202
 * @since 2023-05-18
 */
public class InputStringCheckerTest {
    /**
     * 用例名称：测试字符串为null
     * 前置条件：样例字符串为null, 注解canNull 为false <br/>
     * check点：字符串校验失败<br/>
     */
    @Test
    public void testCanNull() {
        InputStringChecker inputStringChecker = new InputStringChecker();
        inputStringChecker.setCanNull(false);
        inputStringChecker.setType(TypeMode.COMMON);
        inputStringChecker.setMaxLength(100);

        Assert.assertFalse(inputStringChecker.isValid(null, null));
    }

    /**
     * 用例名称：测试字符串可以为blank
     * 前置条件：样例字符串为" ", 注解canNull 为false <br/>
     * check点：字符串校验失败<br/>
     */
    @Test
    public void testCanBlank() {
        InputStringChecker inputStringChecker = new InputStringChecker();
        inputStringChecker.setCanNull(false);
        inputStringChecker.setCanBlank(false);
        inputStringChecker.setType(TypeMode.COMMON);
        inputStringChecker.setMaxLength(100);

        Assert.assertFalse(inputStringChecker.isValid(" ", null));
    }

    /**
     * 用例名称：测试字符串长度
     * 前置条件：样例字符串为" ", 注解canNull 为false，注解设置字符串长度为5；样例字符串长度超过5<br/>
     * check点：字符串校验失败<br/>
     */
    @Test
    public void testStrLength() {
        InputStringChecker inputStringChecker = new InputStringChecker();
        inputStringChecker.setCanNull(false);
        inputStringChecker.setCanBlank(false);
        inputStringChecker.setType(TypeMode.COMMON);
        inputStringChecker.setMaxLength(5);

        Assert.assertFalse(inputStringChecker.isValid("uyiwhjdiuoweojdskuoiqnadsoq", null));
    }


    /**
     * 用例名称：测试正则表达式
     * 前置条件：样例字符串为" ", 注解canNull 为false <br/>
     * check点：字符串校验成功<br/>
     */
    @Test
    public void testReg() {
        InputStringChecker inputStringChecker = new InputStringChecker();
        inputStringChecker.setCanNull(false);
        inputStringChecker.setCanBlank(false);
        // inputStringChecker.setType(TypeMode.COMMON);
        inputStringChecker.setMaxLength(100);
        inputStringChecker.setRegexp(RegexpConstants.NAME_STR);

        Assert.assertTrue(inputStringChecker.isValid("张三zhangsan87923897923", null));
    }

    /**
     * 用例名称：测试黑名单字符
     * 前置条件：样例字符串为" ", 注解canNull 为false;TypeMode.COMMON <br/>
     * check点：字符串校验失败<br/>
     */
    @Test
    public void testBlankChar() {
        InputStringChecker inputStringChecker = new InputStringChecker();
        inputStringChecker.setCanNull(false);
        inputStringChecker.setCanBlank(false);
        inputStringChecker.setType(TypeMode.COMMON);
        inputStringChecker.setMaxLength(100);
        inputStringChecker.setRegexp(RegexpConstants.NAME_STR);

        Assert.assertFalse(inputStringChecker.isValid("张三zhangsan87923897923#", null));
    }

    /**
     * 用例名称：测试黑名单字符
     * 前置条件：样例字符串为" ", 注解canNull 为false;TypeMode.COMMON;字符串无特殊字符 <br/>
     * check点：字符串校验成功<br/>
     */
    @Test
    public void testBlankChar_no_blankChar() {
        InputStringChecker inputStringChecker = new InputStringChecker();
        inputStringChecker.setCanNull(false);
        inputStringChecker.setCanBlank(false);
        inputStringChecker.setType(TypeMode.COMMON);
        inputStringChecker.setMaxLength(100);
        inputStringChecker.setRegexp(RegexpConstants.NAME_STR);

        Assert.assertTrue(inputStringChecker.isValid("张三zhangsan87923897923", null));
    }
}
