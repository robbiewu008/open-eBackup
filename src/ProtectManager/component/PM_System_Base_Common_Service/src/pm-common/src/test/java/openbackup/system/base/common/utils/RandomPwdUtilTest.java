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

import openbackup.system.base.common.utils.RandomPwdUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.security.SecureRandom;

/**
 * 测试随机密码生成工具类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({SecureRandom.class})
public class RandomPwdUtilTest {
    /**
     * 用例场景：输入正确的长度，至少为4
     * 前置条件：输入正确的长度，至少为4
     * 检查点：生成的随机密码长度满足要求
     */
    @Test
    public void generate_pwd_success() {
        // 长度最小为4
        for (int len = 4; len < 10; len++) {
            String pwdTen = RandomPwdUtil.generate(len);
            Assert.assertEquals(pwdTen.length(), len);
        }
    }

    @Test
    public void generate_pwd_legal() {
        String pwdTen = RandomPwdUtil.generate(12);
        while (RandomPwdUtil.isReduplicate(pwdTen)) {
            pwdTen = RandomPwdUtil.generate(12);
        }
        Assert.assertFalse(RandomPwdUtil.isReduplicate(pwdTen));
    }
}
