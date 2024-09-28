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
package openbackup.sqlserver.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * SQL Server单实例注册测试类
 *
 */
@RunWith(PowerMockRunner.class)
public class SqlServerInstanceProviderTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    private SqlServerInstanceProvider sqlServerInstanceProvider;

    private ProviderManager providerManager;

    private ResourceService resourceService;

    @Mock
    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    @Before
    public void init() {
        this.providerManager = Mockito.mock(ProviderManager.class);
        PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
        this.resourceService = Mockito.mock(ResourceService.class);
        SqlServerBaseService sqlServerBaseService = Mockito.mock(SqlServerBaseService.class);
        this.sqlServerInstanceProvider = new SqlServerInstanceProvider(this.providerManager, pluginConfigManager,
            this.resourceService, sqlServerBaseService);
    }

    /**
     * 用例场景：SQL Server实例检查类provider过滤
     * 前置条件：资源类型为SQL Server单实例
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        Assert.assertTrue(sqlServerInstanceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：SQL Server实例检查类provider过滤
     * 前置条件：资源类型为非SQL Server单实例
     * 检查点：类过滤检查返回失败
     */
    @Test
    public void when_resource_type_not_equal_then_applicable_return_false() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDB.getType());
        Assert.assertFalse(sqlServerInstanceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：SQL Server实例创建
     * 前置条件：联通性检查成功、且未创建过改实例
     * 检查点：SQL Server实例创建成功
     */
    @Test
    public void beforeCreate_success() {
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        PageListResponse<ProtectedResource> protectedResource = new PageListResponse<>();
        protectedResource.setRecords(Collections.singletonList(new ProtectedResource()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(protectedResource);
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(resource))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        sqlServerInstanceProvider.beforeCreate(resource);
        Assert.assertEquals(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(), resource.getSubType());
    }

    /**
     * 用例场景：SQL Server实例更新
     * 前置条件：联通性检查成功
     * 检查点：SQL Server实例更新成功
     */
    @Test
    public void before_update_success() {
        ProtectedResource resource = getProtectedResource();
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(resource))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        sqlServerInstanceProvider.beforeUpdate(resource);
        String path = resource.getName() + SqlServerConstants.RESOURCE_NAME_SPLIT + resource.getName();
        Assert.assertEquals(path, resource.getPath());
    }

    /**
     * 用例场景：SQL Server实例创建
     * 前置条件：联通性检查成功，资源已经存在
     * 检查点：SQL Server实例创建失败
     */
    @Test
    public void should_success_when_DataProtectionAccessException_when_beforeCreate() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        List<ProtectedResource> records = new ArrayList<>();
        ProtectedResource resource = getProtectedResource();
        records.add(resource);
        pageListResponse.setRecords(records);
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(resource))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        sqlServerInstanceProvider.beforeCreate(resource);
        Assert.assertTrue(true);
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("sqlserver_resource");
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        Authentication auth = new Authentication();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "1433");
        auth.setExtendInfo(extendInfo);
        resource.setAuth(auth);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("123456");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, Collections.singletonList(protectedResource));
        resource.setDependencies(dependencies);
        resource.setExtendInfo(extendInfo);
        return resource;
    }
}
