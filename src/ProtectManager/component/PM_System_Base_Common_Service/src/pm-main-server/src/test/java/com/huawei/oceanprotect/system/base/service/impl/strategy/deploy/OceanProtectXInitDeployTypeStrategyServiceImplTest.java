/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024 All rights reserved.
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