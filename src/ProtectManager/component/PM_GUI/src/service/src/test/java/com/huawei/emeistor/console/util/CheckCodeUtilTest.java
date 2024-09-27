/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.emeistor.console.util;

import org.junit.Assert;
import org.junit.Test;

/**
 * CheckCodeUtil llt
 *
 * @author g00500588
 * @version: [OceanProtect X8000 1.2.1]
 * @since 2022/08/24
 */
public class CheckCodeUtilTest {

    /**
     * 测试场景：生成验证码
     * 前置条件：无
     * 检查点：生成验证码成功
     */
    @Test
    public void test_run_verify_code_success() {
        int length = 4;
        String verifyCode = CheckCodeUtil.runVerifyCode(length);
        Assert.assertEquals(verifyCode.length(), length);
    }
}