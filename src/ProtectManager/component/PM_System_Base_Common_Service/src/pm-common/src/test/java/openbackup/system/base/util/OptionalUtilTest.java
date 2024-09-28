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
package openbackup.system.base.util;

import openbackup.system.base.util.OptionalUtil;

import org.junit.Assert;
import org.junit.Test;

import java.util.Optional;

/**
 * Optional Util Test
 *
 */
public class OptionalUtilTest {
    /**
     * 用例名称：验证match方法匹配逻辑是否正确。<br/>
     * 前置条件：Optional对象初始化完成。<br/>
     * check点：<br/>
     * 1、正常将类型匹配的元数过滤出来；<br/>
     */
    @Test
    public void test_match() {
        Assert.assertFalse(Optional.of(0).flatMap(OptionalUtil.match(String.class)).isPresent());
        Assert.assertTrue(Optional.of("1").flatMap(OptionalUtil.match(CharSequence.class)).isPresent());
    }
}
