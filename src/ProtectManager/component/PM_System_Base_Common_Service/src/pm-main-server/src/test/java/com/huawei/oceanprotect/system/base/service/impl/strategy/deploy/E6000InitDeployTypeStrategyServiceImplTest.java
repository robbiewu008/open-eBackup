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
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.impl.pacific.PacificInitNetworkServiceImpl;

import openbackup.system.base.sdk.system.model.StorageAuth;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.web.multipart.MultipartFile;

/**
 * E6000InitDeployTypeStrategyServiceImplTest
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-23
 */
@RunWith(PowerMockRunner.class)
public class E6000InitDeployTypeStrategyServiceImplTest {
    @InjectMocks
    private E6000InitDeployTypeStrategyServiceImpl e6000InitDeployTypeStrategyService;

    @Mock
    private InitConfigService initConfigService;

    @Mock
    private PacificInitNetworkServiceImpl pacificInitNetworkService;

    /**
     * 用例场景：解析lld
     * 前置条件：正常运行
     * 检查点：成功
     */
    @Test
    public void test_getInitNetworkBodyByLLD_success() {
        StorageAuth storageAuth = new StorageAuth("admin", "xxxx");
        PowerMockito.when(initConfigService.getLocalStorageAuth()).thenReturn(storageAuth);

        PowerMockito.when(initConfigService.getLocalStorageDeviceId()).thenReturn("123");

        MultipartFile multipartFile = ResourceHelper.createMultipartFile(getClass(), "LLD_test.xls");
        InitNetworkBody initNetworkBodyByLLD = e6000InitDeployTypeStrategyService.getInitNetworkBodyByLLD(
            multipartFile);
        Assert.assertEquals(3, initNetworkBodyByLLD.getBackupNetworkConfig()
            .getPacificInitNetWorkInfoList().size());
        Assert.assertEquals(3, initNetworkBodyByLLD.getArchiveNetworkConfig()
            .getPacificInitNetWorkInfoList().size());
    }
}