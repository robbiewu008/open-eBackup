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
package openbackup.oracle.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
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
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

public class OracleSingleNodeDatabaseProviderTest {
    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final InstanceResourceService instanceResourceService = Mockito.mock(InstanceResourceService.class);
    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);
    private final AgentUnifiedService restApi = Mockito.mock(AgentUnifiedService.class);
    private final UnifiedConnectionCheckProvider unifiedConnectionCheckProvider = Mockito.mock(
            UnifiedConnectionCheckProvider.class);

    private OracleSingleNodeDatabaseProvider databaseProvider;


    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        databaseProvider = new OracleSingleNodeDatabaseProvider(resourceService, providerManager,
                instanceResourceService, oracleBaseService, restApi);
    }

    /**
     * 用例场景：oracle实例检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.ORACLE.getType());
        Assert.assertTrue(databaseProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：注册单机时是否需要检查重名
     * 前置条件：无
     * 检查点：ShouldCheckResourceNameDuplicate 为 false
     */
    @Test
    public void should_return_false_when_get_resource_feature() {
        Assert.assertFalse(databaseProvider.getResourceFeature().isShouldCheckResourceNameDuplicate());
    }

    /**
     * 用例场景：oracle实例创建
     * 前置条件：联通性检查成功、且未创建过改实例
     * 检查点：oracle实例创建成功
     */
    @Test
    public void before_create_success() {
        ProtectedResource resource = getProtectedResource();
        resource.getExtendInfo().put("name","hwdb");
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(55536);
        resource.setEnvironment(environment);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource)).thenReturn(mockContext());
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put("inst_name","sda");
        Mockito.when(restApi.getClusterInfo(any(),any())).thenReturn(appEnvResponse);
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        databaseProvider.beforeCreate(resource);
        Assert.assertEquals(ResourceSubTypeEnum.ORACLE.getType(), resource.getSubType());
        Assert.assertNull(resource.getUserId());
    }

    /**
     * 用例场景：oracle实例创建
     * 前置条件：联通性检查成功、存储快照校验
     * 检查点：oracle实例创建成功
     */
    @Test
    public void before_create_success_when_storage() {
        ProtectedResource resource = getProtectedResource();
        resource.getExtendInfo().put("name","hwdb");
        String storages = "[{\"username\":\"name\",\"password\":\"pwd\",\"ipList\":\"8.40.100.35,8.40.100.36\","
            + "\"port\":1,"
            + "\"isRegister\":"
            + " \"false\",\"enableCert\": \"0\",\"certName\": \"caCertificate_1703241958297.pem\",\"certSize\": \"5.5KB\",\"crlName\": \"\",\"crlSize\": \"\"}]";
        resource.getExtendInfo().put("storages", storages);
        resource.getAuth().getExtendInfo().put("storages", storages);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(55536);
        resource.setEnvironment(environment);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(mockContext());
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put("inst_name","sda");
        Mockito.when(restApi.getClusterInfo(any(),any())).thenReturn(appEnvResponse);
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        databaseProvider.beforeCreate(resource);
        Assert.assertEquals(ResourceSubTypeEnum.ORACLE.getType(), resource.getSubType());
        Assert.assertNull(resource.getUserId());
    }

    /**
     * 用例场景：oracle实例创建
     * 前置条件：联通性检查成功、且未创建过改实例，且实例不是顶级实例
     * 检查点：oracle实例创建成功
     */
    @Test
    public void before_create_success_when_is_not_top_instance() {
        ProtectedResource resource = getProtectedResource();
        resource.getExtendInfo().put("name","hwdb");
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(55536);
        resource.setEnvironment(environment);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put("inst_name","sda");
        Mockito.when(restApi.getClusterInfo(any(),any())).thenReturn(appEnvResponse);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource)).thenReturn(mockContext());
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        databaseProvider.beforeCreate(resource);
        Mockito.verify(oracleBaseService, Mockito.times(1)).getEnvironmentById(any());
    }

    @Test(expected = LegoCheckedException.class)
    public void before_create_fail_when_storage_over_limit(){
        ProtectedResource resource = getProtectedResource();
        Map<String, String> extendInfo = resource.getExtendInfo();
        extendInfo.put("storages", "[{'username':'admin','password':'Admin@storage1','port':'8088','ip':'8.40.98.92',"
            + "'enableCert':'0'},{'username':'admin','password':'Admin@storage1','port':'8088','ip':'8.40.98.92',\"\n"
            + "            + \"'enableCert':'0'}]");
        resource.setExtendInfo(extendInfo);
        databaseProvider.beforeCreate(resource);
    }

    @Test(expected = LegoCheckedException.class)
    public void before_create_fail_when_ip_format_error(){
        ProtectedResource resource = getProtectedResource();
        Map<String, String> extendInfo = resource.getExtendInfo();
        extendInfo.put("storages", "[{'username':'admin','password':'Admin@storage1','port':'8088','ip':'5555.665.3.3',"
            + "'enableCert':'0'}]");
        resource.setExtendInfo(extendInfo);
        databaseProvider.beforeCreate(resource);
    }

    @Test(expected = DataProtectionAccessException.class)
    public void before_create_fail_when_storage_connect_success(){
        ProtectedResource resource = getProtectedResource();
        Map<String, String> extendInfo = resource.getExtendInfo();
        extendInfo.put("storages", "[{'username':'admin','password':'Admin@storage1','port':'8088','ip':'127.0.0.1',"
            + "'enableCert':'0'}]");
        resource.setExtendInfo(extendInfo);
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(mockContext());
        databaseProvider.beforeCreate(resource);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：oracle实例更新
     * 前置条件：联通性检查成功、数据库在线
     * 检查点：oracle实例更新成功
     */
    @Test
    public void before_update_success() {
        ProtectedResource resource = getProtectedResource();
        resource.getExtendInfo().put("name","hwdb");
        List<ProtectedResource> agents = new ArrayList<>();
        resource.setDependencies(new HashMap<>());
        agents.add(new ProtectedEnvironment());
        resource.getDependencies().put(DatabaseConstants.AGENTS, agents);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(555366);
        resource.setEnvironment(environment);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
            .thenReturn(mockContext());
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        databaseProvider.beforeUpdate(resource);
        Mockito.verify(oracleBaseService,
            Mockito.times(1)).refreshClusterInstanceActiveStandby(any(), any());
    }

    /**
     * 用例场景：oracle实例更新失败
     * 前置条件：联通性检查不成功
     * 检查点：oracle实例更新失败
     */
    @Test
    public void should_throw_LegoCheckedException_before_update_if_context_is_empty() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource resource = getProtectedResource();
        Mockito.when(providerManager.findProvider(any(), any()))
            .thenReturn(unifiedConnectionCheckProvider);
        ResourceCheckContext context = new ResourceCheckContext();
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(context);
        databaseProvider.beforeUpdate(resource);
    }

    /**
     * 用例场景：oracle实例更新失败
     * 前置条件：联通性检查不成功
     * 检查点：oracle实例更新失败
     */
    @Test
    public void should_throw_LegoCheckedException_before_update_if_action_result_code_not_correct() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource resource = getProtectedResource();
        Mockito.when(providerManager.findProvider(any(), any()))
            .thenReturn(unifiedConnectionCheckProvider);
        ResourceCheckContext context = new ResourceCheckContext();
        List<ActionResult> list = new ArrayList<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1L);
        actionResult.setBodyErr("99");
        list.add(actionResult);
        context.setActionResults(list);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(context);
        databaseProvider.beforeUpdate(resource);
    }

    /**
     * 用例场景：oracle实例更新
     * 前置条件：联通性检查成功、刷新子实例状态失败
     * 检查点：oracle实例更新失败
     */
    @Test
    public void should_throw_LegoCheckedException_before_update_if_dependencies_is_empty() {
        ProtectedResource resource = getProtectedResource();
        resource.getExtendInfo().put("name","hwdb");
        List<ProtectedResource> agents = new ArrayList<>();
        resource.setDependencies(new HashMap<>());
        agents.add(new ProtectedEnvironment());
        resource.getDependencies().put(DatabaseConstants.AGENTS, agents);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(555366);
        resource.setEnvironment(environment);

        ResourceCheckContext context = mockContext();
        context.getActionResults().get(0).setCode(-1);
        context.getActionResults().get(0).setBodyErr("-1");
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
            .thenReturn(context);
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        ProtectedResource resource1 = getProtectedResource();
        resource1.getExtendInfo().put("name","hwdb");
        resource1.setDependencies(new HashMap<>());
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource1));
        Assert.assertThrows(LegoCheckedException.class, () -> databaseProvider.beforeUpdate(resource));
    }

    /**
     * 用例场景：oracle实例创建失败
     * 前置条件：获取agent信息失败，获取Agent信息错误
     * 检查点：报错
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_before_create_when_agent_error() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(55536);
        resource.setEnvironment(environment);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
            .thenReturn(mockContext());
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        databaseProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：oracle实例创建失败
     * 前置条件：获取的agent信息扩展字段是空
     * 检查点：报错
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_before_create_when_agent_extend_info_null() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(55536);
        resource.setEnvironment(environment);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
            .thenReturn(mockContext());
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        AppEnvResponse appEnvResponse = getAppEnvResponse();
        appEnvResponse.setExtendInfo(null);
        databaseProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：oracle实例创建
     * 前置条件：oracle服务没有启动
     * 检查点：报错
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_oracle_not_running_when_before_create() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource resource = getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.emptyList());
        Mockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resource))
            .thenReturn(unifiedConnectionCheckProvider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.99.110");
        environment.setPort(55536);
        resource.setEnvironment(environment);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(resource))
            .thenReturn(mockContext());
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(new ProtectedEnvironment()));
        AppEnvResponse appEnvResponse = getAppEnvResponse();
        appEnvResponse.getExtendInfo().put("status", "1");
        databaseProvider.beforeCreate(resource);
    }

    @Test
    public void should_throw_LegoCheckedException_if_action_result_is_empty() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource protectedResource = getProtectedResource();
        Mockito.when(providerManager.findProvider(any(),any())).thenReturn(unifiedConnectionCheckProvider);
        ResourceCheckContext context = new ResourceCheckContext();
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        databaseProvider.beforeCreate(protectedResource);
    }

    @Test
    public void should_throw_LegoCheckedException_if_action_result_detail_params_is_empty() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("check connection failed.");
        ProtectedResource protectedResource = getProtectedResource();
        Mockito.when(providerManager.findProvider(any(),any())).thenReturn(unifiedConnectionCheckProvider);
        ResourceCheckContext context = new ResourceCheckContext();
        ActionResult actionResult = new ActionResult();
        actionResult.setBodyErr("1000");
        actionResult.setCode(-1);
        context.setActionResults(Collections.singletonList(actionResult));
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        databaseProvider.beforeCreate(protectedResource);
    }

    @Test
    public void should_throw_LegoCheckedException_if_action_result_detail_params_is_not_empty() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("check connection failed.");
        ProtectedResource protectedResource = getProtectedResource();
        Mockito.when(providerManager.findProvider(any(),any())).thenReturn(unifiedConnectionCheckProvider);
        ResourceCheckContext context = new ResourceCheckContext();
        ActionResult actionResult = new ActionResult();
        List<String> params=new ArrayList<>();
        params.add("db");
        params.add("asd");
        actionResult.setDetailParams(params);
        actionResult.setBodyErr("1000");
        actionResult.setCode(-1);
        context.setActionResults(Collections.singletonList(actionResult));
        Mockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        databaseProvider.beforeCreate(protectedResource);
    }


    /**
     * 用例场景：当接口返回任务不可删除时，任务删除失败
     * 前置条件：接口返回任务不可删除
     * 检查点: 抛出异常
     */
    @Test
    public void test_delete_pre_handle_failed(){
        PowerMockito.when(oracleBaseService.isAnonymizationDeletable(anyString())).thenReturn(false);
        Assert.assertThrows(LegoCheckedException.class,
                () -> databaseProvider.preHandleDelete(getProtectedResource()));
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.ORACLE.getType());
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
        appEnvResponse.getExtendInfo().put(OracleConstants.ORACLE_HOME,"das");
        appEnvResponse.getExtendInfo().put(OracleConstants.IS_ASM_INST,"1");
        appEnvResponse.getExtendInfo().put(OracleConstants.DB_ROLE,"2");
        appEnvResponse.getExtendInfo().put(OracleConstants.INST_NAME,"hwdb");
        appEnvResponse.getExtendInfo().put(OracleConstants.ASM_INFO,"");
        appEnvResponse.getExtendInfo().put("status", "0");
        appEnvResponse.getExtendInfo().put(DatabaseConstants.VERSION, "5.5");
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
