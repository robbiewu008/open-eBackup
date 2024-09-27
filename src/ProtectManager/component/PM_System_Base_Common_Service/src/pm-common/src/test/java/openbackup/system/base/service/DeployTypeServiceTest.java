/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.service;

import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.config.configmap.ConfigMapService;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * Session Service Test
 *
 * @author c30016231
 * @since 2022-06-10
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({DeployTypeService.class})
public class DeployTypeServiceTest {
    @InjectMocks
    private DeployTypeService deployTypeService;

    @Mock
    private ConfigMapService configMapService;

    /**
     * 用例名称：验证通过环境变量获取x8000的部署类型。<br/>
     * 前置条件：环境变量存在DEPLOY_TYPE。<br/>
     * check点：<br/>
     * 返回x8000类型的部署类型枚举；<br/>
     */
    @Test
    public void test_get_x8000_deploy_type_from_system_env() {
        PowerMockito.mockStatic(System.class);
        PowerMockito.when(System.getenv("DEPLOY_TYPE")).thenReturn("d0");
        DeployTypeEnum deployType = deployTypeService.getDeployType();
        Assert.assertEquals(deployType, DeployTypeEnum.X8000);
    }

    /**
     * 用例名称：验证通过configMap获取x8000的部署类型。<br/>
     * 前置条件：configMap存在DEPLOY_TYPE。<br/>
     * check点：<br/>
     * 返回x8000类型的部署类型枚举；<br/>
     */
    @Test
    public void test_get_x8000_deploy_type_from_config_map() {
        PowerMockito.when(configMapService.getDeployType()).thenReturn("d0");
        DeployTypeEnum deployType = deployTypeService.getDeployType();
        Assert.assertEquals(deployType, DeployTypeEnum.X8000);
    }

    /**
     * 用例名称：验证获取是否为cloudBackup类型为True。<br/>
     * 前置条件：configMap存在为d3的部署类型。<br/>
     * check点：<br/>
     * 返回值为true；<br/>
     */
    @Test
    public void test_is_cloudBackup_true() {
        PowerMockito.when(configMapService.getDeployType()).thenReturn("d3");
        boolean isCloudBackup = deployTypeService.isCloudBackup();
        Assert.assertTrue(isCloudBackup);
    }

    /**
     * 用例名称：验证当环境变量中的类型为其他类型时抛出异常。<br/>
     * 前置条件：环境变量DEPLOY_TYPE类型为xx。<br/>
     * check点：<br/>
     * 抛出IllegalArgumentException异常；<br/>
     */
    @Test
    public void test_should_throw_IllegalArgumentException_if_env_param_is_wrong_when_get_deploy_type() {
        PowerMockito.mockStatic(System.class);
        PowerMockito.when(System.getenv("DEPLOY_TYPE")).thenReturn("xx");
        Assert.assertThrows(IllegalArgumentException.class,() -> deployTypeService.getDeployType());
    }

    /**
     * 用例名称：检查部署类型是否支持lld初始化
     * 前置条件：正常运行
     * check点：成功
     */
    @Test
    public void test_isSupportInitByLLD_when_support_then_return_true() {
        PowerMockito.mockStatic(System.class);
        PowerMockito.when(System.getenv("DEPLOY_TYPE")).thenReturn("d0");
        boolean supportInitByLLD = deployTypeService.isSupportInitByLLD();
        Assert.assertTrue(supportInitByLLD);
    }

    /**
     * 用例名称：检查部署类型是否支持lld初始化
     * 前置条件：正常运行
     * check点：成功
     */
    @Test
    public void test_isSupportInitByLLD_when_not_support_then_return_false() {
        PowerMockito.mockStatic(System.class);
        PowerMockito.when(System.getenv("DEPLOY_TYPE")).thenReturn("d3");
        boolean supportInitByLLD = deployTypeService.isSupportInitByLLD();
        Assert.assertFalse(supportInitByLLD);
    }
}
