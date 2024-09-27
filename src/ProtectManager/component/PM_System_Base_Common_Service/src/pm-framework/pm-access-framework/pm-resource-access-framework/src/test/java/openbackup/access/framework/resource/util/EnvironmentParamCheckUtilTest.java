/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import org.junit.Assert;
import org.junit.Test;

import java.util.regex.Pattern;

/**
 * 检查环境名称工具测试类
 *
 * @author x30038064
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/12/13
 */
public class EnvironmentParamCheckUtilTest {
    /**
     * 用例名称：检验名字规则是否为空
     * 前置条件：字符串
     * check点：字符为空，抛出参数异常
     */
    @Test
    public void check_env_name_empty_success() {
        Assert.assertThrows(LegoCheckedException.class, () -> EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(""));
        Assert.assertThrows(LegoCheckedException.class,
            () -> EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(null));
    }

    /**
     * 用例名称：检验名字规则是否符合要求
     * 前置条件：字符串
     * check点：字符超过64个或者不符合pattern，抛出参数异常
     */
    @Test
    public void check_env_name_not_pattern_success() {
        Assert.assertThrows("environment name is not pattern", LegoCheckedException.class,
            () -> EnvironmentParamCheckUtil.checkEnvironmentNamePattern("123456"));
        Assert.assertThrows("environment name is not pattern", LegoCheckedException.class,
            () -> EnvironmentParamCheckUtil.checkEnvironmentNamePattern(
                "lllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll"));
    }

    /**
     * 用例名称：检验IP地址是否符合要求
     * 前置条件：字符串
     * check点：输入本地回环地址，输入0.0.0.0 输入 255.255.255.255广播地址报错
     */
    @Test
    public void check_env_endpoint_valid_throw_LegoException() {
        Assert.assertThrows("endpoint is not valid", LegoCheckedException.class,
            () -> EnvironmentParamCheckUtil.checkValidIP("127.0.0.1"));
        Assert.assertThrows("endpoint is not valid", LegoCheckedException.class,
            () -> EnvironmentParamCheckUtil.checkValidIP("0.0.0.0"));
        Assert.assertThrows("endpoint is not valid", LegoCheckedException.class,
            () -> EnvironmentParamCheckUtil.checkValidIP("255.255.255.255"));
    }

    /**
     * 校验IP地址成功
     */
    @Test
    public void check_env_endpoint_valid_success() {
        EnvironmentParamCheckUtil.checkValidIP("192.168.1.1");
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：检验参数校验正则表达式
     * 前置条件：字符串
     * check点：不符合表达式抛出参数异常
     */
    @Test
    public void check_env_name_pattern_success() {
        Assert.assertThrows(LegoCheckedException.class, () -> checkEnvNamePattern("1name"));
        Assert.assertThrows(LegoCheckedException.class, () -> checkEnvNamePattern("name$$$"));
        Assert.assertThrows(LegoCheckedException.class, () -> checkEnvNamePattern("name$$$"));
        Assert.assertThrows(LegoCheckedException.class, () -> checkEnvNamePattern("_name$$$"));
        checkEnvNamePattern("");
        checkEnvNamePattern("name");
        checkEnvNamePattern("名称111");
    }

    private void checkEnvNamePattern(String name) {
        if (!Pattern.matches(RegexpConstants.ENV_NAME_PATTERN, name)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "environment name is illegal");
        }
    }
}
