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
package openbackup.dameng.protection.access.util;

import openbackup.dameng.protection.access.util.DamengParamCheckUtil;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;

/**
 * {@link DamengParamCheckUtil test}
 *
 */
public class DamengParamCheckUtilTest {
    /**
     * 用例场景：dameng注册，检测端口是否合法
     * 前置条件：端口合法
     * 检查点：检测通过
     */
    @Test
    public void check_port_success() {
        DamengParamCheckUtil.checkPort("65535");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：dameng注册，检测端口是否合法
     * 前置条件：端口不合法
     * 检查点：检测不通过
     */
    @Test
    public void should_throw_LegoCheckedException_when_port_is_illegal() {
        Assert.assertThrows(LegoCheckedException.class, () -> DamengParamCheckUtil.checkPort("65536"));
    }

    /**
     * 用例场景：dameng注册，检测数据库用户名是否合法
     * 前置条件：用户名合法
     * 检查点：检测通过
     */
    @Test
    public void check_database_username_success() {
        DamengParamCheckUtil.checkAuthKey("test");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：dameng注册，检测数据库用户名是否合法
     * 前置条件：用户名不合法
     * 检查点：检测不通过
     */
    @Test
    public void should_throw_LegoCheckedException_when_database_username_is_illegal() {
        Assert.assertThrows(LegoCheckedException.class, () -> DamengParamCheckUtil.checkAuthKey("12345"));
    }
}

