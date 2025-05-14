/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
 * @author n30046257
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
