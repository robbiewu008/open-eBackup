/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.database.base.plugin.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;

import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Collections;

/**
 * The DatabaseResourceProviderTest
 *
 * @author g30003063
 * @since 2022/5/31
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    DatabaseEnvironmentProvider.class, DatabaseResourceProvider.class, ProviderManager.class
})
public class DatabaseResourceProviderTest {
    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private DatabaseResourceProvider databaseResourceProvider;

    @Autowired
    private DatabaseEnvironmentProvider databaseEnvironmentProvider;

    @MockBean
    private PluginConfigManager pluginConfigManager;

    /**
     * 用例名称：默认场景下，获取ResourceProvider成功
     * 前置条件：配置插件配置文件后
     * check点：获取到的ResourceProvider是DatabaseResourceProvider
     */
    @Test
    @Ignore
    public void get_database_resource_provider_success() {
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setType(ResourceTypeEnum.DATABASE.getType());
        pluginConfig.setSubType("mock_sub_type");
        Mockito.when(pluginConfigManager.getPluginConfigs()).thenReturn(Collections.singletonList(pluginConfig));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType("mock_sub_type");
        ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, protectedResource);
        Assert.assertEquals(provider, databaseResourceProvider);
    }

    /**
     * 用例名称：默认场景下，获取EnvironmentProvider成功
     * 前置条件：配置插件配置文件后
     * check点：获取到的EnvironmentProvider是DatabaseEnvironmentProvider
     */
    @Test
    public void get_database_environment_provider_success() {
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setType(ResourceTypeEnum.DATABASE.getType());
        pluginConfig.setSubType("mock_sub_type");
        Mockito.when(pluginConfigManager.getPluginConfigs()).thenReturn(Collections.singletonList(pluginConfig));
        EnvironmentProvider provider = providerManager.findProvider(EnvironmentProvider.class, "mock_sub_type");
        Assert.assertEquals(provider, databaseEnvironmentProvider);
    }
}
