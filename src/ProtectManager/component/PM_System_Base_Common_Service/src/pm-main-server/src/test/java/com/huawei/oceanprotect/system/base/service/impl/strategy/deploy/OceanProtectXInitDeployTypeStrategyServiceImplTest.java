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
package com.huawei.oceanprotect.system.base.service.impl.strategy.deploy;

import com.huawei.oceanprotect.system.base.ResourceHelper;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeNetworkBodyXlsAbility;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.web.multipart.MultipartFile;

/**
 * OceanProtectXInitDeployTypeStrategyServiceImplTest
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-24
 */
@RunWith(PowerMockRunner.class)
public class OceanProtectXInitDeployTypeStrategyServiceImplTest {
    @InjectMocks
    private OceanProtectXInitDeployTypeStrategyServiceImpl oceanProtectXInitDeployTypeStrategyService;

    @Mock
    private InitializeNetworkBodyXlsAbility initializeNetworkBodyXlsAbility;

    /**
     * 用例场景：解析lld
     * 前置条件：正常运行
     * 检查点：成功
     */
    @Test
    public void test_getInitNetworkBodyByLLD_success() {
        MultipartFile multipartFile = ResourceHelper.createMultipartFile(getClass(), "LLD_test.xls");
        oceanProtectXInitDeployTypeStrategyService.getInitNetworkBodyByLLD(multipartFile);
        Assert.assertTrue(true);
    }
}