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
package openbackup.cnware.protection.access.util;

import openbackup.cnware.protection.access.util.CnwareUtil;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-21
 */
@RunWith(PowerMockRunner.class)
public class CnwareUtilTest {
    /**
     * 用例场景：检查环境域名信息
     * 前置条件：域名中含有非法字符
     * 检查点：若域名中含有非法字符则应抛出异常
     */
    @Test
    public void test_checkEndpoint_should_throw_exception_when_domian_name_not_match() {
        Assert.assertThrows(LegoCheckedException.class, () -> CnwareUtil.checkEndpoint("dsaf%daf"));
    }


    /**
     * 用例场景：检查环境名称
     * 前置条件：环境名称中含有非法字符
     * 检查点：若环境名称中含有非法字符则应抛出异常
     */
    @Test
    public void test_environment_name_should_throw_exception_when_environment_name_is_not_match() {
        Assert.assertThrows(LegoCheckedException.class, () -> CnwareUtil.verifyEnvName("dsaf%daf"));
    }
}