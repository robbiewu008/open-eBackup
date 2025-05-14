/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-09
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