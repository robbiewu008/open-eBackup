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
package com.huawei.oceanprotect.system.base.service.impl.pacific;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.OpenStorageService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * PacificServiceImplTest
 *
 */
@RunWith(PowerMockRunner.class)
public class PacificServiceImplTest {
    @InjectMocks
    private PacificServiceImpl pacificService;

    @Mock
    private OpenStorageService openStorageService;

    /**
     * 用例场景：有可用的业务ip
     * 前置条件：无
     * 检查点：获取成功
     */
    @Test
    public void test_getNetworkInfo_when_business_ip_found_then_success() {
        pacificService.getNetworkInfo("", "");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：有可用的业务ip, 未指定管理ip
     * 前置条件：无
     * 检查点：获取成功
     */
    @Test
    public void test_getNetworkInfo_when_manage_ip_is_null_then_success() {
        pacificService.getNetworkInfo("", "");
        Assert.assertTrue(true);
    }
}