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
package openbackup.system.base.common.validator.constants;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import org.junit.Assert;
import org.junit.Test;

import java.util.regex.Pattern;

/**
 * 常数项验证
 *
 * @author t30021437
 * @since 2023-04-03
 */
public class RegexpConstantsTest {
    /**
     * 用例场景:校验新增字符.
     * 前置条件：无
     * 检查点字符加点成功
     */
    @Test
    public void modify_pattern_success() {
        String oldPattern = "^[a-zA-Z0-9_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$";
        boolean isMatches = isClusterNamePattern(oldPattern, "test.");
        Assert.assertFalse(isMatches);
        isMatches = isClusterNamePattern(RegexpConstants.NAME_STR, "test.");
        Assert.assertTrue(isMatches);
    }

    private boolean isClusterNamePattern(String matcher, String name) {
        if (Pattern.matches(matcher, name)) {
            return true;
        }
        return false;
    }
}
