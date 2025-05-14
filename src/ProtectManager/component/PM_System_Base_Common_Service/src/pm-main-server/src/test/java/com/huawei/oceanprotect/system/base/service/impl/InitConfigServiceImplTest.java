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
package com.huawei.oceanprotect.system.base.service.impl;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.sdk.system.model.StorageAuth;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.List;

/**
 * 测试
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/2/25
 */
public class InitConfigServiceImplTest {
    private final InitNetworkConfigMapper initNetworkConfigMapper = Mockito.mock(InitNetworkConfigMapper.class);
    private final EncryptorService encryptorService = Mockito.mock(EncryptorService.class);

    private final InitConfigServiceImpl initConfigService = new InitConfigServiceImpl(initNetworkConfigMapper,
            encryptorService);

    /**
     * 用例场景：更新数据库信息成功
     * 前置条件：NA
     * 检查点：无
     */
    @Test
    public void should_update_local_storageAuth_success() {
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername("wuyanzu");
        storageAuth.setPassword("zhendeshuai");
        initConfigService.updateLocalStorageAuth(storageAuth);
    }

    /**
     * 用例场景：获取数据库中存储信息成功
     * 前置条件：NA
     * 检查点：校验存储信息
     */
    @Test
    public void should_get_local_storageAuth_success() {
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername("wuyanzu");
        storageAuth.setPassword("zhendeshuai");
        List<InitConfigInfo> initConfigInfos = new ArrayList<>();
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitValue(
                "{\"username\":\"admin\",\"password\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAkVgHRmejwbI1m1KC0rhjQ56nSfSm+g3fA25DgUAAAAAAAAAAAAAAAAAAAAHtUZ0YObmzYPoojdt2ktXwPmcYMGa3wRruwCHNrlZQ==\"}");
        initConfigInfo.setInitType("storageAuth");
        initConfigInfos.add(initConfigInfo);
        Mockito.when(initNetworkConfigMapper.queryInitConfig(anyString())).thenReturn(initConfigInfos);
        Mockito.when(encryptorService.decrypt(anyString())).thenReturn("zhendeshuai");
        initConfigService.getLocalStorageAuth();
        Assert.assertEquals("zhendeshuai", initConfigService.getLocalStorageAuth().getPassword());
    }
}
