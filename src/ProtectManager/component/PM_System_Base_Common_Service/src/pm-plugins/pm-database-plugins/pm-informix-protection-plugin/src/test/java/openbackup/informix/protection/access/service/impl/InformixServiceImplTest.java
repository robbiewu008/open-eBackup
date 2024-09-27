/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.informix.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentCheckProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionRejectException;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.informix.protection.access.constant.InformixConstant;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * InformixServiceImplTest
 *
 * @author zWX951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-17
 */
public class InformixServiceImplTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final UnifiedEnvironmentCheckProvider unifiedCheckProvider =
            PowerMockito.mock(UnifiedEnvironmentCheckProvider.class);

    private final ClusterEnvironmentService clusterEnvironmentService =
            PowerMockito.mock(ClusterEnvironmentService.class);

    private final InstanceProtectionService instanceProtectionService =
            PowerMockito.mock(InstanceProtectionService.class);

    private final InformixService informixService = PowerMockito.mock(InformixService.class);

    private final EnvironmentServices environmentServices = new EnvironmentServices(unifiedCheckProvider,
            clusterEnvironmentService);

    private final InformixServiceImpl informixServiceImpl = new InformixServiceImpl(resourceService,
            agentUnifiedService, providerManager, instanceProtectionService, environmentServices);

    /**
     * 用例场景：检查agent 环境信息
     * 前置条件：无
     * 检查点：agent response返回为空时报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_agentResources_is_null() {
        ProtectedResource instance = new ProtectedResource();
        instance.setDependencies(new HashMap<>());
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.queryAgentEnvironment(instance));
    }

    /**
     * 用例场景：查询agent 环境信息成功
     * 前置条件：无
     * 检查点：获取到agent信息
     */
    @Test
    public void queryAgentEnvironment_success() {
        ProtectedResource instance = new ProtectedResource();
        List<ProtectedResource> agentResources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("uuid");
        agentResources.add(protectedResource);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<String, List<ProtectedResource>>() {{
            put(DatabaseConstants.AGENTS, agentResources);
        }};
        instance.setDependencies(dependencies);
        PowerMockito.when(resourceService.getResourceById(anyString()))
                .thenReturn(Optional.of(new ProtectedEnvironment()));
        Assert.assertNotNull(informixServiceImpl.queryAgentEnvironment(instance));
    }

    /**
     * 用例场景：检查日志备份信息
     * 前置条件：无
     * 检查点：日志备份开关开启
     */
    @Test
    public void checkLogBackupItem_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, String> extendInfo = new HashMap<String, String>() {{
            put("logBackup", "0");
            put("logBackupPath", "");
        }};
        protectedEnvironment.setExtendInfo(extendInfo);
        informixServiceImpl.checkLogBackupItem(protectedEnvironment);
        String backupStatus = protectedEnvironment.getExtendInfo().getOrDefault(InformixConstant.LOG_BACKUP, "");
        Assert.assertEquals(InformixConstant.LOG_BACKUP_OFF, backupStatus);
    }

    /**
     * 用例场景：检查日志备份信息
     * 前置条件：无
     * 检查点：日志备份路径无效时抛异常
     */
    @Test
    public void should_throw_DataProtectionRejectException_if_logBackupPath_is_invalid() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, String> extendInfo = new HashMap<String, String>() {{
            put("logBackup", "1");
            put("logBackupPath", "");
        }};
        protectedEnvironment.setExtendInfo(extendInfo);
        Assert.assertThrows(DataProtectionRejectException.class, () ->
                informixServiceImpl.checkLogBackupItem(protectedEnvironment));
    }

    /**
     * 用例场景：检查agent信息
     * 前置条件：无
     * 检查点：agent信息无效时抛异常
     */
    @Test
    public void should_throw_DataProtectionRejectException_if_agentsList_size_is_invalid() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<String, List<ProtectedResource>>() {{
            put(DatabaseConstants.AGENTS, new ArrayList<>());
        }};
        protectedEnvironment.setDependencies(dependencies);
        Assert.assertThrows(DataProtectionRejectException.class, () ->
                informixServiceImpl.checkHostInfo(protectedEnvironment));
    }

    /**
     * 用例场景：agent 状态是否在线
     * 前置条件：无
     * 检查点：离线报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_status_is_offline() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<String, List<ProtectedResource>>() {{
            put(DatabaseConstants.AGENTS, Collections.singletonList(new ProtectedResource()));
        }};
        protectedEnvironment.setDependencies(dependencies);
        PowerMockito.when(resourceService.getResourceById(anyString()))
                .thenReturn(Optional.of(new ProtectedEnvironment()));
        Assert.assertThrows(LegoCheckedException.class, () ->
                informixServiceImpl.checkHostInfo(protectedEnvironment));
    }

    /**
     * 用例场景：agent response信息检查
     * 前置条件：无
     * 检查点：agent response返回为空时报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_agentresponse_is_empty() {
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>();
        protectedResourcePageListResponse.setTotalCount(0);
        protectedResourcePageListResponse.setRecords(new ArrayList<>());
        PowerMockito.when(agentUnifiedService.getDetailPageList(any(), any(), any(), any()))
                .thenReturn(protectedResourcePageListResponse);
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.listResource(
                new ProtectedResource(), new ProtectedEnvironment(), "Informix-service"));
    }

    /**
     * 用例场景：注册实例成功
     * 前置条件：无
     * 检查点：返回一条成功信息
     */
    @Test
    public void list_Resource_success() {
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>();
        protectedResourcePageListResponse.setTotalCount(1);
        protectedResourcePageListResponse.setRecords(Collections.singletonList(new ProtectedResource()));
        PowerMockito.when(agentUnifiedService.getDetailPageList(any(), any(), any(), any()))
                .thenReturn(protectedResourcePageListResponse);
        PageListResponse<ProtectedResource> protectedResourcePageListResponse1 = informixServiceImpl.listResource(
                new ProtectedResource(), new ProtectedEnvironment(), "Informix-service");
        Assert.assertEquals(1, protectedResourcePageListResponse1.getTotalCount());
    }

    /**
     * 用例场景：agent返回结果为空抛异常
     * 前置条件：无
     * 检查点：为空抛异常
     */
    @Test
    public void should_throw_DataProtectionAccessException_if_agent_response_is_not_empty() {
        ProtectedResource resource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource agentProtectedResource = new ProtectedResource();
        agentProtectedResource.setUuid("uuid");
        dependency.put(DatabaseConstants.AGENTS, Collections.singletonList(agentProtectedResource));
        resource.setDependencies(dependency);
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>();
        protectedResourcePageListResponse.setTotalCount(1);
        protectedResourcePageListResponse.setRecords(Collections.singletonList(new ProtectedResource()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(),
                ArgumentMatchers.anyMap(), any())).thenReturn(protectedResourcePageListResponse);
        Assert.assertThrows(DataProtectionAccessException.class, () ->
                informixServiceImpl.checkInstanceExist(resource));
    }

    /**
     * 用例场景：更新resource信息
     * 前置条件：无
     * 检查点：更新成功
     */
    @Test
    public void update_Resource_success() {
        // resource
        ProtectedResource resource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource childResource = new ProtectedResource();
        ProtectedResource agentProtectedResource = new ProtectedResource();
        agentProtectedResource.setUuid("uuid");
        agentProtectedResource.setEndpoint("test_point");
        PowerMockito.when(resourceService.getResourceById(anyString()))
            .thenReturn(Optional.of(agentProtectedResource));
        childResource.setDependencies(new HashMap<String, List<ProtectedResource>>() {{
            put("agents", Collections.singletonList(agentProtectedResource));
        }});
        childResource.setExtendInfo(new HashMap<>());
        dependency.put(DatabaseConstants.CHILDREN, Collections.singletonList(childResource));
        resource.setDependencies(dependency);
        // clusterExtendInfo
        Map<String, String> map = new HashMap<String, String>() {{
            put(InformixConstant.APPLICATION_VERSION, "v1");
            put(InformixConstant.INSTANCESTATUS, "INSTANCESTATUS");
            put(InformixConstant.PAIRED_SERVER_IP, "PAIRED_SERVER_IP");
            put(InformixConstant.PAIRED_SERVER, "PAIRED_SERVER");
            put(InformixConstant.LOCAL_SERVER, "LOCAL_SERVER");
            put(InformixConstant.SERVER_NUM, "SERVER_NUM");
            put(InformixConstant.ROOT_DBS_PATH, "ROOT_DBS_PATH");
        }};
        Map<String, Map<String, String>> clusterExtendInfo = new HashMap<String, Map<String, String>>() {{
            put("uuid", map);
        }};
        informixServiceImpl.updateResource(resource, clusterExtendInfo);
        Assert.assertEquals("v1", resource.getVersion());
        Assert.assertEquals("test_point", resource.getPath());
    }

    /**
     * 用例场景：生成主备集群实例信息列表
     * 前置条件：无
     * 检查点：生成成功
     */
    @Test
    public void get_response_list_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        resource.setEnvironment(new ProtectedEnvironment());
        List<ProtectedResource> agentsList = new ArrayList<>();
        agentsList.add(resource);
        Map<String, List<ProtectedResource>> map = new HashMap<>();
        map.put(DatabaseConstants.AGENTS, agentsList);
        ProtectedResource resource1 = new ProtectedResource();
        resource.setSubType("Informix-singleInstance");
        resource1.setDependencies(map);
        map.put(DatabaseConstants.CHILDREN, Collections.singletonList(resource1));
        resource.setDependencies(map);
        ResourceConnectionCheckProvider provider = PowerMockito.mock(ResourceConnectionCheckProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        ResourceCheckContext context = new ResourceCheckContext();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        context.setActionResults(Collections.singletonList(actionResult));
        PowerMockito.when(provider.tryCheckConnection(any())).thenReturn(context);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setDependencies(new HashMap<>());
        protectedResource.getDependencies().put(DatabaseConstants.AGENTS, agentsList);
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(InformixConstant.AGENT_IP_LIST, "127.0.0.1");
        protectedResource.setExtendInfo(extendInfo);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setPort(22);
        environment.setEndpoint("127.0.0.1");
        PowerMockito.when(resourceService.getResourceById(any()))
                .thenReturn(Optional.of(protectedResource), Optional.of(protectedResource), Optional.of(environment));
        PageListResponse<ProtectedResource> responses = new PageListResponse<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource recordResource = new ProtectedResource();
        recordResource.setExtendInfo(new HashMap<>());
        resources.add(recordResource);
        responses.setRecords(resources);
        PowerMockito.when(agentUnifiedService.getDetailPageList(anyString(), anyString(), anyInt(), any()))
                .thenReturn(responses);
        Assert.assertNotNull(informixServiceImpl.getResponsesList(resource, false));
    }

    /**
     * 用例场景：生成ListResourceV2Req信息
     * 前置条件：无
     * 检查点：生成成功
     */
    @Test
    public void generate_list_resource_V2Req_success() throws Exception {
        Assert.assertNotNull(
                Whitebox.invokeMethod(informixServiceImpl, "generateListResourceV2Req", new ProtectedEnvironment(),
                        new ProtectedResource()));
    }

    /**
     * 用例场景：通过id获取environment信息
     * 前置条件：无
     * 检查点：获取不到抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_get_environment_by_id_when_resource_is_not_environment() {
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(new ProtectedResource()));
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.getEnvironmentById("uuid"));
    }

    /**
     * 用例场景：检查连通性是否联通
     * 前置条件：无
     * 检查点：不符合结果要求抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_context_result_is_not_success_code() {
        ResourceConnectionCheckProvider provider = PowerMockito.mock(ResourceConnectionCheckProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        ResourceCheckContext context = new ResourceCheckContext();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(1);
        PowerMockito.when(provider.tryCheckConnection(any())).thenReturn(context);
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.checkInstanceConnection(new ProtectedResource()));
    }

    /**
     * 用例场景：检查连通性结果
     * 前置条件：无
     * 检查点：不符合结果要求抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_context_result_is_empty() {
        ResourceConnectionCheckProvider provider = PowerMockito.mock(ResourceConnectionCheckProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.when(provider.tryCheckConnection(any())).thenReturn(new ResourceCheckContext());
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.checkInstanceConnection(new ProtectedResource()));
    }

    /**
     * 用例场景：检查实例连通性
     * 前置条件：无
     * 检查点：是否联通
     */
    @Test
    public void check_instance_connection_success() {
        ResourceConnectionCheckProvider provider = PowerMockito.mock(ResourceConnectionCheckProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        ResourceCheckContext context = new ResourceCheckContext();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        context.setActionResults(Collections.singletonList(actionResult));
        PowerMockito.when(provider.tryCheckConnection(any())).thenReturn(context);
        informixServiceImpl.checkInstanceConnection(new ProtectedResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查集群服务连通性
     * 前置条件：无
     * 检查点：是否联通
     */
    @Test
    public void check_Service_Connection_success() {
        EnvironmentCheckProvider provider = PowerMockito.mock(EnvironmentCheckProvider.class);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(provider);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        informixServiceImpl.checkServiceConnection(environment);
        verify(provider, times(1)).check(environment);
    }

    /**
     * 用例场景：获取主备集群信息成功
     * 前置条件：无
     * 检查点：获取信息是否成功
     */
    @Test
    public void get_cluster_info_success() {
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(InformixConstant.INSTANCESTATUS, InformixConstant.MASTER_NODE_STATUS);
        extendInfo1.put(InformixConstant.APPLICATION_VERSION, "version");
        extendInfo1.put(InformixConstant.PAIRED_SERVER, "server2");
        extendInfo1.put(InformixConstant.LOCAL_SERVER, "server1");
        extendInfo1.put(InformixConstant.AGENT_IP_LIST, "ip1, ip2");
        extendInfo1.put(InformixConstant.PAIRED_SERVER_IP, "ip1");
        resource.setExtendInfo(extendInfo1);
        resource.setUuid("uuid1");
        response1.setRecords(Collections.singletonList(resource));
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        ProtectedResource resource2 = new ProtectedResource();
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put(InformixConstant.INSTANCESTATUS, InformixConstant.SECOND_NODE_STATUS);
        extendInfo2.put(InformixConstant.APPLICATION_VERSION, "version");
        extendInfo2.put(InformixConstant.PAIRED_SERVER, "server1");
        extendInfo2.put(InformixConstant.LOCAL_SERVER, "server2");
        extendInfo2.put(InformixConstant.AGENT_IP_LIST, "ip1, ip2");
        extendInfo2.put(InformixConstant.PAIRED_SERVER_IP, "ip2");
        resource2.setExtendInfo(extendInfo2);
        resource2.setUuid("uuid2");
        response2.setRecords(Collections.singletonList(resource2));
        responsesList.add(response1);
        responsesList.add(response2);
        Map<String, Map<String, String>> clusterExtendInfoMap = informixServiceImpl.getClusterExtendInfo(
                responsesList);
        Assert.assertEquals(extendInfo1, clusterExtendInfoMap.get("uuid1"));
    }

    /**
     * 用例场景：检查主备集群的extendInfo1信息失败
     * 前置条件：无
     * 检查点：信息不匹配时抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_cluster_extendInfo_node1_info_checked_failed() {
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(InformixConstant.INSTANCESTATUS, InformixConstant.SECOND_NODE_STATUS);
        resource.setExtendInfo(extendInfo1);
        response1.setRecords(Collections.singletonList(resource));
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        ProtectedResource resource2 = new ProtectedResource();
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put(InformixConstant.INSTANCESTATUS, InformixConstant.SECOND_NODE_STATUS);
        resource2.setExtendInfo(extendInfo2);
        response2.setRecords(Collections.singletonList(resource2));
        responsesList.add(response1);
        responsesList.add(response2);
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.getClusterExtendInfo(responsesList));
    }

    /**
     * 用例场景：检查主备集群的extendInfo2信息失败
     * 前置条件：无
     * 检查点：信息不匹配时抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_cluster_extendInfo_node2_info_checked_failed() {
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(InformixConstant.INSTANCESTATUS, InformixConstant.MASTER_NODE_STATUS);
        resource.setExtendInfo(extendInfo1);
        response1.setRecords(Collections.singletonList(resource));
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        ProtectedResource resource2 = new ProtectedResource();
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put(InformixConstant.INSTANCESTATUS, InformixConstant.MASTER_NODE_STATUS);
        resource2.setExtendInfo(extendInfo2);
        response2.setRecords(Collections.singletonList(resource2));
        responsesList.add(response1);
        responsesList.add(response2);
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.getClusterExtendInfo(responsesList));
    }

    /**
     * 用例场景：检测集群的extendinfo信息是否匹配
     * 前置条件：无
     * 检查点：不匹配时抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_cluster_extendInfo_node_status_is_not_match() {
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(InformixConstant.INSTANCESTATUS, "OFF-Line");
        resource.setExtendInfo(extendInfo1);
        response1.setRecords(Collections.singletonList(resource));
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        ProtectedResource resource2 = new ProtectedResource();
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put(InformixConstant.INSTANCESTATUS, "OFF-Line");
        resource2.setExtendInfo(extendInfo2);
        response2.setRecords(Collections.singletonList(resource2));
        responsesList.add(response1);
        responsesList.add(response2);
        Assert.assertThrows(LegoCheckedException.class, () -> informixServiceImpl.getClusterExtendInfo(responsesList));
    }

    /**
     * 用例场景：检测集群的extendinfo信息是否匹配
     * 前置条件：无
     * 检查点：匹配时不抛出异常
     */
    @Test
    public void get_cluster_extendInfo_node_status_success() {
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(InformixConstant.INSTANCESTATUS, "Updatable (Sec)");
        resource.setExtendInfo(extendInfo1);
        response1.setRecords(Collections.singletonList(resource));
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        ProtectedResource resource2 = new ProtectedResource();
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put(InformixConstant.INSTANCESTATUS, "On-Line (Prim)");
        resource2.setExtendInfo(extendInfo2);
        response2.setRecords(Collections.singletonList(resource2));
        responsesList.add(response1);
        responsesList.add(response2);
        Map<String, Map<String, String>> clusterExtendInfo = informixServiceImpl.getClusterExtendInfo(responsesList);
        Assert.assertEquals(clusterExtendInfo.size(), 1);
    }

    /**
     * 用例场景：注册单机实例时成功
     * 前置条件：无
     * 检查点：是否注册成功
     */
    @Test
    public void register_single_instance_success() {
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(new HashMap<>());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, protectedResources);
        resource.setDependencies(dependencies);
        resource.setUuid("uuid");
        resource.setSubType("Informix-singleInstance");
        protectedResources.add(resource);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setPort(22);
        environment.setEndpoint("127.0.0.1");
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(environment));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource resource1 = new ProtectedResource();
        Map<String, String> agentExtendInfo = new HashMap<>();
        agentExtendInfo.put(InformixConstant.APPLICATION_VERSION, "version");
        agentExtendInfo.put(InformixConstant.SERVER_NUM, "num");
        agentExtendInfo.put(InformixConstant.ROOT_DBS_PATH, "rootdbsPath");
        resource1.setExtendInfo(agentExtendInfo);
        List<ProtectedResource> list = new ArrayList<>();
        list.add(resource1);
        response.setRecords(list);
        PowerMockito.when(agentUnifiedService.getDetailPageList(anyString(), anyString(), anyInt(), any()))
                .thenReturn(response);
        informixServiceImpl.registerSingleInstance(resource, resource);
        Assert.assertEquals("version", resource.getVersion());
        Assert.assertEquals("num", resource.getExtendInfoByKey(InformixConstant.SERVER_NUM));
        Assert.assertEquals("rootdbsPath", resource.getExtendInfoByKey(InformixConstant.ROOT_DBS_PATH));
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
                resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    /**
     * 用例场景：注册单机实例时，agent信息是否超过一条
     * 前置条件：无
     * 检查点：超过一条抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_register_single_instance_agent_is_not_one() {
        List<ProtectedResource> protectedResources = new ArrayList<>();
        protectedResources.add(new ProtectedResource());
        protectedResources.add(new ProtectedResource());
        ProtectedResource resource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, protectedResources);
        resource.setDependencies(dependencies);
        Assert.assertThrows(LegoCheckedException.class,
                () -> informixServiceImpl.registerSingleInstance(resource, resource));
    }

    /**
     * 用例场景：检查主备集群实例信息是否匹配
     * 前置条件：无
     * 检查点：信息不匹配抛异常
     */
    @Test
    public void check_instance_match_success() throws Exception {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(InformixConstant.APPLICATION_VERSION, "version");
        extendInfo.put(InformixConstant.PAIRED_SERVER, "server2");
        extendInfo.put(InformixConstant.LOCAL_SERVER, "server1");
        extendInfo.put(InformixConstant.AGENT_IP_LIST, "ip1, ip2");
        extendInfo.put(InformixConstant.PAIRED_SERVER_IP, "ip1");
        extendInfo.put(InformixConstant.INSTANCESTATUS, InformixConstant.MASTER_NODE_STATUS);
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put(InformixConstant.APPLICATION_VERSION, "version");
        extendInfo2.put(InformixConstant.PAIRED_SERVER, "server1");
        extendInfo2.put(InformixConstant.LOCAL_SERVER, "server2");
        extendInfo2.put(InformixConstant.AGENT_IP_LIST, "ip1, ip2");
        extendInfo2.put(InformixConstant.PAIRED_SERVER_IP, "ip2");
        extendInfo2.put(InformixConstant.INSTANCESTATUS, InformixConstant.SECOND_NODE_STATUS);
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setExtendInfo(extendInfo);
        ProtectedResource protectedResource2 = new ProtectedResource();
        protectedResource2.setExtendInfo(extendInfo2);
        PageListResponse<ProtectedResource> protectedResourcePageListResponse1 = new PageListResponse<>();
        PageListResponse<ProtectedResource> protectedResourcePageListResponse2 = new PageListResponse<>();
        protectedResourcePageListResponse1.setTotalCount(1);
        protectedResourcePageListResponse1.setRecords(Collections.singletonList(protectedResource1));
        protectedResourcePageListResponse2.setTotalCount(1);
        protectedResourcePageListResponse2.setRecords(Collections.singletonList(protectedResource2));
        responsesList.add(protectedResourcePageListResponse1);
        responsesList.add(protectedResourcePageListResponse2);
        Whitebox.invokeMethod(informixServiceImpl, "checkIfInstanceMatch", responsesList);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：实例信息不匹配时抛异常
     * 前置条件：无
     * 检查点：信息不匹配抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_instance_does_not_match() {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(InformixConstant.APPLICATION_VERSION, "version");
        extendInfo.put(InformixConstant.PAIRED_SERVER, "server2");
        extendInfo.put(InformixConstant.LOCAL_SERVER, "server1");
        extendInfo.put(InformixConstant.AGENT_IP_LIST, "ip1, ip2");
        extendInfo.put(InformixConstant.PAIRED_SERVER_IP, "ip1");
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put(InformixConstant.APPLICATION_VERSION, "version");
        extendInfo2.put(InformixConstant.PAIRED_SERVER, "server1");
        extendInfo2.put(InformixConstant.LOCAL_SERVER, "server2");
        extendInfo2.put(InformixConstant.AGENT_IP_LIST, "ip1, ip2");
        extendInfo2.put(InformixConstant.PAIRED_SERVER_IP, "ip3");
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setExtendInfo(extendInfo);
        ProtectedResource protectedResource2 = new ProtectedResource();
        protectedResource2.setExtendInfo(extendInfo2);
        PageListResponse<ProtectedResource> protectedResourcePageListResponse1 = new PageListResponse<>();
        PageListResponse<ProtectedResource> protectedResourcePageListResponse2 = new PageListResponse<>();
        protectedResourcePageListResponse1.setTotalCount(1);
        protectedResourcePageListResponse1.setRecords(Collections.singletonList(protectedResource1));
        protectedResourcePageListResponse2.setTotalCount(1);
        protectedResourcePageListResponse2.setRecords(Collections.singletonList(protectedResource2));
        responsesList.add(protectedResourcePageListResponse1);
        responsesList.add(protectedResourcePageListResponse2);
        Assert.assertThrows(LegoCheckedException.class,
                () -> Whitebox.invokeMethod(informixServiceImpl, "checkIfInstanceMatch", responsesList));
    }

    /**
     * 用例场景：从map中获取指定的key值
     * 前置条件：无
     * 检查点：所取值是否正确
     */
    @Test
    public void get_target_value_success() throws Exception {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("key", "value");
        String value = Whitebox.invokeMethod(informixServiceImpl, "getTargetValue", extendInfo, "key");
        Assert.assertEquals("value", value);
    }

    /**
     * 用例场景：通过resource获取单机实例env节点信息
     * 前置条件：无
     * 检查点：是否可以取到值
     */
    @Test
    public void get_Env_Nodes_By_Instance_Resource_1_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        informixServiceImpl.getEnvNodesByInstanceResource(resource);
        Mockito.verify(instanceProtectionService, Mockito.times(1)).extractEnvNodesBySingleInstance(resource);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：通过resource获取集群实例env节点信息
     * 前置条件：无
     * 检查点：是否可以取到值
     */
    @Test
    public void get_env_nodes_by_instance_resource_2_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
        informixServiceImpl.getEnvNodesByInstanceResource(resource);
        Mockito.verify(instanceProtectionService, Mockito.times(1)).extractEnvNodesByClusterInstance(resource);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：通过resource获取agent信息
     * 前置条件：无
     * 检查点：是否可以取到值
     */
    @Test
    public void get_agents_by_instance_resource_success() {
        List<TaskEnvironment> agentList = new ArrayList<>();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("uuid");
        taskEnvironment.setEndpoint("endPoint");
        taskEnvironment.setPort(0);
        agentList.add(taskEnvironment);
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
        PowerMockito.when(informixService.getEnvNodesByInstanceResource(resource)).thenReturn(agentList);
        List<Endpoint> agentInfo = informixServiceImpl.getAgentsByInstanceResource(resource);
        Assert.assertEquals(0, agentInfo.size());
    }

    /**
     * 用例场景：通过resource获取agent信息
     * 前置条件：无
     * 检查点：是否可以取到值
     */
    @Test
    public void get_resource_by_id_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType("Database");
        Optional<ProtectedResource> protectedResourceOption = Optional.of(protectedResource);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(protectedResourceOption);
        ProtectedResource resourceById = informixServiceImpl.getResourceById("");
        Assert.assertEquals("Database", resourceById.getType());
    }

    /**
     * 用例场景：检查主机是否安装了informix
     * 前置条件：无
     * 检查点：检查主机是否安装了informix
     */
    @Test
    public void check_application_success() {
        PowerMockito.when(resourceService.getResourceById(anyString()))
                .thenReturn(Optional.of(new ProtectedEnvironment()));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any()))
                .thenReturn(agentBaseDto);
        PowerMockito.when(agentUnifiedService.checkApplicationNoRetry(any(),any()))
            .thenReturn(agentBaseDto);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        HashMap<String, List<ProtectedResource>> dependenceMap = new HashMap<>();
        List<ProtectedResource> agentList = new ArrayList<>();
        ProtectedResource source = new ProtectedResource();
        source.setUuid("123-123-123");
        agentList.add(source);
        dependenceMap.put(DatabaseConstants.AGENTS, agentList);
        protectedEnvironment.setDependencies(dependenceMap);
        try {
            informixServiceImpl.checkApplication(protectedEnvironment);
        } catch (LegoCheckedException exception) {
            Assert.fail();
        }
        Assert.assertTrue(true);
    }
}