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

/**
 * CheckCodeUtil llt
 *
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
