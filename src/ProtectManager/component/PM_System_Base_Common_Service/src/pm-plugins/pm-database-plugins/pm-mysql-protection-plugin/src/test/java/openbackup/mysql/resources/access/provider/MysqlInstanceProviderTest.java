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
package openbackup.mysql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.provider.config.MysqlAgentProxyConfig;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
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
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * MySQL单实例注册测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class})
public class MysqlInstanceProviderTest {
    private MysqlInstanceProvider mysqlInstanceProvider;

    private ProviderManager providerManager;

    private ResourceService resourceService;

    private AgentUnifiedService agentUnifiedService;

    private MysqlBaseService mysqlBaseService = Mockito.mock(MysqlBaseService.class);

    @Mock
    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        this.agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        this.providerManager = Mockito.mock(ProviderManager.class);
        this.resourceService = Mockito.mock(ResourceService.class);
        MysqlAgentProxyConfig agentProxyProperties = new MysqlAgentProxyConfig();
        agentProxyProperties.setHost("10.44.218.91");
        agentProxyProperties.setPort(3306);
        PowerMockito.mockStatic(FeignBuilder.class);
        AgentUnifiedRestApi builder = Mockito.mock(AgentUnifiedRestApi.class);
        PowerMockito.when(
                FeignBuilder.buildHttpsTarget(eq(AgentUnifiedRestApi.class), any(), anyBoolean(), anyBoolean(), any()))
                .thenReturn(builder);
        this.mysqlInstanceProvider = new MysqlInstanceProvider(this.providerManager, mysqlBaseService, resourceService,
                agentUnifiedService);
        mysqlInstanceProvider.setEncryptorService(PowerMockito.mock(EncryptorService.class));
    }

    /**
     * 用例场景：mysql实例检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType());
        Assert.assertTrue(mysqlInstanceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：mysql实例创建
     * 前置条件：联通性检查成功、且未创建过改实例
     * 检查点：mysql实例创建成功
     */
    @Test
    public void beforeCreate_success() {
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
                .thenReturn(unifiedConnectionCheckProvider);

        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(environment);
        PowerMockito.when(mysqlBaseService.queryDatabaseVersion(any(),any())).thenReturn("5.5.5-10.2.43-MariaDB-log");
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
                .thenReturn(mockContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(getAppEnvResponse());

        mysqlInstanceProvider.beforeCreate(resource);
        Assert.assertEquals(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(), resource.getSubType());
        Assert.assertTrue(resource.getUserId() == null);
        Assert.assertTrue(getAppEnvResponse().getExtendInfo()
                .get(DatabaseConstants.IS_MASTER)
                .equals(resource.getExtendInfo().get(DatabaseConstants.ROLE)));
    }

    /**
     * 用例场景：mysql实例创建
     * 前置条件：联通性检查成功、且未创建过改实例，且实例不是顶级实例
     * 检查点：mysql实例创建成功
     */
    @Test
    public void beforeCreate_success_when_is_not_top_instance() {
        ProtectedResource resource = getProtectedResource();
        resource.getExtendInfo().put(DatabaseConstants.IS_TOP_INSTANCE, "1");
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
                .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(environment);
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
                .thenReturn(mockContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(getAppEnvResponse());
        mysqlInstanceProvider.beforeCreate(resource);
        Mockito.verify(mysqlBaseService, Mockito.times(1)).getEnvironmentById(any());
    }

    /**
     * 用例场景：mysql实例创建失败
     * 前置条件：获取agent信息失败，获取Agent信息错误
     * 检查点：报错
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_beforeCreate_when_agent_error() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
                .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(environment);
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
                .thenReturn(mockContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(null);
        mysqlInstanceProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：mysql实例创建失败
     * 前置条件：获取的agent信息扩展字段是空
     * 检查点：报错
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_beforeCreate_when_agent_extend_info_null() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
                .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(environment);
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
                .thenReturn(mockContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        AppEnvResponse appEnvResponse = getAppEnvResponse();
        appEnvResponse.setExtendInfo(null);
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);
        mysqlInstanceProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：mysql实例创建
     * 前置条件：mysql服务没有启动
     * 检查点：报错
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_mysql_not_running_when_before_create() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
                .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(environment);
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
                .thenReturn(mockContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        AppEnvResponse appEnvResponse = getAppEnvResponse();
        appEnvResponse.getExtendInfo().put("status", "1");
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);
        mysqlInstanceProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：mysql实例创建
     * 前置条件：MySQL的log bin日志路径查询失败
     * 检查点：报错
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_mysql_log_bin_index_is_null_when_before_create() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
                .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(environment);
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource)).thenReturn(mockContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        AppEnvResponse appEnvResponse = getAppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put(DatabaseConstants.IS_MASTER, "1");
        appEnvResponse.getExtendInfo().put("status", "0");
        appEnvResponse.getExtendInfo().put(DatabaseConstants.DATA_DIR, "/data");
        appEnvResponse.getExtendInfo().put(DatabaseConstants.VERSION, "5.5");
        appEnvResponse.getExtendInfo().put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "red hat");
        appEnvResponse.getExtendInfo().put(MysqlConstants.MYSQL_SERVICE_NAME, "mysql.service");
        appEnvResponse.getExtendInfo().put(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE, "systemctl");
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);
        mysqlInstanceProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：mysql实例创建
     * 前置条件：联通性检查成功，资源已经存在
     * 检查点：mysql实例创建失败
     */
    @Test
    public void should_throw_DataProtectionAccessException_when_beforeCreate() {
        expectedException.expect(DataProtectionAccessException.class);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        List<ProtectedResource> records = new ArrayList<>();
        ProtectedResource resource = getProtectedResource();
        records.add(resource);
        pageListResponse.setRecords(records);
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
                .thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
                .thenReturn(mockContext());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        mysqlInstanceProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：设置是否更新dependence中host配置为false
     * 前置条件：无
     * 检查点: 设置成功
     */
    @Test
    public void getResourceFeature_success() {
        Assert.assertFalse(mysqlInstanceProvider.getResourceFeature().isShouldUpdateDependencyHostInfoWhenScan());
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType());
        Authentication auth = new Authentication();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "3306");
        extendInfo.put(DatabaseConstants.IS_TOP_INSTANCE, "0");
        auth.setExtendInfo(extendInfo);
        resource.setAuth(auth);
        HashMap<String, String> hashMap = new HashMap<>();
        resource.setExtendInfo(hashMap);
        return resource;
    }

    private AppEnvResponse getAppEnvResponse() {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put(DatabaseConstants.IS_MASTER, "1");
        appEnvResponse.getExtendInfo().put("status", "0");
        appEnvResponse.getExtendInfo().put(DatabaseConstants.DATA_DIR, "/data");
        appEnvResponse.getExtendInfo().put(DatabaseConstants.VERSION, "5.5");
        appEnvResponse.getExtendInfo().put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "red hat");
        appEnvResponse.getExtendInfo().put(MysqlConstants.MYSQL_SERVICE_NAME, "mysql.service");
        appEnvResponse.getExtendInfo().put(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE, "systemctl");
        appEnvResponse.getExtendInfo().put(MysqlConstants.LOG_BIN_INDEX_PATH, "/log");
        return appEnvResponse;
    }

    private ResourceCheckContext mockContext() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        List<ActionResult> list = new ArrayList<>();
        list.add(actionResult);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(list);
        return context;
    }
}
