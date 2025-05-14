/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl.strategy.deploy;

import com.huawei.oceanprotect.system.base.service.strategy.deploy.InitDeployTypeStrategyService;

import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * InitDeployTypeStrategyContextTest
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-21
 */
@SpringBootTest(classes = {InitDeployTypeStrategyContext.class})
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
public class InitDeployTypeStrategyContextTest {
    @Autowired
    private InitDeployTypeStrategyContext initDeployTypeStrategyContext;

    @MockBean
    private DeployTypeService deployTypeService;

    /**
     * 用例场景：根据部署类型获取具体策略实现类
     * 前置条件：程序正常运行
     * 检查点：获取成功
     */
    @Test
    public void test_getStrategyService_success() {
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E1000);
        InitDeployTypeStrategyService strategyService = initDeployTypeStrategyContext.getStrategyService();
        Assert.assertTrue(strategyService instanceof E1000InitDeployTypeStrategyServiceImpl);

        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X8000);
        InitDeployTypeStrategyService strategyService1 = initDeployTypeStrategyContext.getStrategyService();
        Assert.assertTrue(strategyService1 instanceof OceanProtectXInitDeployTypeStrategyServiceImpl);

        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);
        InitDeployTypeStrategyService strategyService2 = initDeployTypeStrategyContext.getStrategyService();
        Assert.assertTrue(strategyService2 instanceof E6000InitDeployTypeStrategyServiceImpl);

        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.CLOUD_BACKUP);
        InitDeployTypeStrategyService strategyService3 = initDeployTypeStrategyContext.getStrategyService();
        Assert.assertTrue(strategyService3 instanceof DefaultInitDeployTypeStrategyServiceImpl);
    }
}