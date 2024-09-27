/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.enums.OpenGaussClusterStateEnum;
import openbackup.opengauss.resources.access.util.OpenGaussClusterUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
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
import java.util.regex.Pattern;

/**
 * OpenGauss环境注册提供者测试类
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-20
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {ResourceCheckContextUtil.class, OpenGaussClusterUtil.class, EnvironmentLinkStatusHelper.class})
public class OpenGaussDatabaseEnvironmentProviderTest {
    private ProviderManager providerManager;

    private ResourceConnectionCheckProvider resource;

    private PluginConfigManager pluginConfigManager;

    private OpenGaussDatabaseEnvironmentProvider openGaussEnvironmentProvider;

    private AgentUnifiedService agentUnifiedService;

    private ResourceService resourceService;

    private ProtectedEnvironmentService environmentService;

    private JsonSchemaValidator jsonSchemaValidator;

    @Before
    public void init() {
        resource = PowerMockito.mock(ResourceConnectionCheckProvider.class);
        environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        providerManager = PowerMockito.mock(ProviderManager.class);
        pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);
        agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        resourceService = PowerMockito.mock(ResourceService.class);
        jsonSchemaValidator = PowerMockito.mock(JsonSchemaValidator.class);
        openGaussEnvironmentProvider = new OpenGaussDatabaseEnvironmentProvider(providerManager, pluginConfigManager,
            resourceService, jsonSchemaValidator, agentUnifiedService);
        openGaussEnvironmentProvider.setEnvironmentService(environmentService);
    }

    /**
     * 用例场景 集群注册成功，扫描集群下面的数据库和表信息
     * 前置条件：集群环境注册正常
     * 检查点: 如果是单机扫描，直接下发
     */
    @Test
    public void should_scan_single_node_if_agent_is_singele_when_scan() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, clusterEnvironment))
            .thenReturn(resource);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("8.3.6.63");
        protectedEnvironment.setPort(8088);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedEnvironment));
        AgentUnifiedRestApi agentUnifiedRestApi = PowerMockito.mock(AgentUnifiedRestApi.class);
        PowerMockito.when(agentUnifiedRestApi.getDetail(any(), any(), any())).thenReturn(mockAgentResourceResponse());
        List<ProtectedResource> protectedResources = openGaussEnvironmentProvider.scan(clusterEnvironment);
        Assert.assertEquals(protectedResources.size(), 2);
        Assert.assertEquals("8.3.6.63", protectedResources.get(0).getPath());
    }

    private static AgentDetailDto mockAgentResourceResponse() {
        AgentDetailDto response = new AgentDetailDto();
        List<AppResource> appResources = new ArrayList<>();
        AppResource mockOpenGauss = new AppResource();
        mockOpenGauss.setName("mockOpenGauss1");
        mockOpenGauss.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        mockOpenGauss.setType(ResourceTypeEnum.DATABASE.getType());

        AppResource mockNamespace = new AppResource();
        mockNamespace.setName("mockOpenGauss2");
        mockNamespace.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        mockOpenGauss.setType(ResourceTypeEnum.DATABASE.getType());

        appResources.add(mockOpenGauss);
        appResources.add(mockNamespace);
        response.setResourceList(appResources);
        return response;
    }

    /**
     * 用例场景 集群注册成功，扫描集群下面的数据库和表信息
     * 前置条件：集群环境注册正常
     * 检查点: 如果是集群扫描，主节点下发请求，如果找不到主节点抛出异常
     */
    @Test
    public void should_should_Throw_LegoCheckedException_if_agent_is_cluster_when_scan() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironmentTwoAnent();
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, clusterEnvironment))
            .thenReturn(resource);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("8.3.6.63");
        protectedEnvironment.setPort(8088);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedEnvironment));
        AgentUnifiedRestApi agentUnifiedRestApi = PowerMockito.mock(AgentUnifiedRestApi.class);
        PowerMockito.when(agentUnifiedRestApi.getDetail(any(), any(), any())).thenReturn(mockAgentResourceResponse());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> openGaussEnvironmentProvider.scan(clusterEnvironment));
        Assert.assertEquals("No available agent information was found.", legoCheckedException.getMessage());
    }

    private ProtectedEnvironment getMasterAngentTow() {
        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setUuid(OpenGaussMockData.agentId3);
        agent2.setEndpoint("8.3.6.5");
        agent2.setPort(9636);
        agent2.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        return agent2;
    }

    private Map<String, List<ProtectedResource>> getPrimaryAndStandbyDependency() {
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        List<ProtectedResource> agentsList = new ArrayList<>();
        ProtectedEnvironment standbyAgent = getProtectedEnvironment();
        ProtectedEnvironment primaryAgent = getMasterAngentTow();
        agentsList.add(standbyAgent);
        agentsList.add(primaryAgent);
        dependency.put(DatabaseConstants.AGENTS, agentsList);
        return dependency;
    }

    private ProtectedEnvironment getClusterEnvironmentTwoAnent() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, String> extendInfoNodes = new HashMap<>();
        extendInfoNodes.put(OpenGaussConstants.NODES,
            JsonUtil.json(Collections.singletonList(OpenGaussMockData.getNodeInfo())));
        protectedEnvironment.setExtendInfo(extendInfoNodes);

        Authentication auth = new Authentication();
        auth.setAuthKey("omm");
        auth.setAuthType(Authentication.OS_PASSWORD);
        protectedEnvironment.setAuth(auth);
        protectedEnvironment.setDependencies(getPrimaryAndStandbyDependency());
        protectedEnvironment.setType(ResourceTypeEnum.DATABASE.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        protectedEnvironment.setEndpoint("8.3.6.63");
        return protectedEnvironment;
    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment environment = getClusterEnvironment();
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, environment))
            .thenReturn(resource);
        PowerMockito.when(resource.tryCheckConnection(any())).thenReturn(new ResourceCheckContext());
        PowerMockito.mockStatic(OpenGaussClusterUtil.class);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(OpenGaussClusterUtil.getContextClusterInfo(any()))
            .thenReturn(OpenGaussMockData.buildAppEnvResponseClusterStateUnavailable());
        openGaussEnvironmentProvider.validate(environment);
        Mockito.verify(resourceService, Mockito.times(1)).updateSourceDirectly(any());
    }

    /**
     * 用例场景：健康检查
     * 前置条件：主机离线
     * 检查点: 检查失败，主机离线
     */
    @Test
    public void health_check_failure() {
        ProtectedEnvironment clusterEnvironmentOfflineAgentEnv = getClusterEnvironmentOfflineAgentEnv();
        PowerMockito.when(
                providerManager.findProvider(ResourceConnectionCheckProvider.class, clusterEnvironmentOfflineAgentEnv))
            .thenReturn(resource);
        PowerMockito.when(resource.tryCheckConnection(any())).thenReturn(new ResourceCheckContext());
        PowerMockito.mockStatic(OpenGaussClusterUtil.class);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        PowerMockito.when(OpenGaussClusterUtil.getContextClusterInfo(any()))
            .thenReturn(OpenGaussMockData.buildAppEnvResponseClusterStateUnavailable());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> openGaussEnvironmentProvider.validate(clusterEnvironmentOfflineAgentEnv));
        Assert.assertEquals("host is offline,update the environment status to Offline",
            legoCheckedException.getMessage());
    }

    /**
     * 用例场景：openGauss集群环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(openGaussEnvironmentProvider.applicable(ResourceSubTypeEnum.OPENGAUSS.getType()));
    }

    /**
     * 用例场景 openGauss集群环境注册
     * 前置条件：环境信息参数正确,集群数量已达到2000个
     * 检查点: 检查是否抛出集群数量已达上限的异常
     */
    @Test
    public void should_throw_LegoChecked_Exception_when_resource_exceed_max_count() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        List<ProtectedResource> resources = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            resources.add(new ProtectedResource());
        }
        response.setRecords(resources);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        Assert.assertThrows(LegoCheckedException.class, () -> openGaussEnvironmentProvider.register(clusterEnvironment));
    }

    /**
     * 用例场景 openGauss集群环境注册
     * 前置条件：环境信息参数正确
     * 检查点: 首次注册环境id为空，插件设置唯一的uuid值
     * 集群id生成规则：OpenGauss + ip地址排序 + 用户名代表集群的唯一id
     * 入参相同，集群的uuid唯一 "e40c888d-fa29-3da3-b3f0-847545d00410"
     */
    @Test
    public void should_return_uuid_check_success_when_environment_uuid_is_null() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, clusterEnvironment))
            .thenReturn(resource);
        PowerMockito.when(resource.checkConnection(clusterEnvironment))
            .thenReturn(OpenGaussMockData.mockResourceCheckContextCluster());
        PowerMockito.mockStatic(ResourceCheckContextUtil.class);
        PowerMockito.mockStatic(OpenGaussClusterUtil.class);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(OpenGaussClusterUtil.getContextClusterInfo(any()))
            .thenReturn(OpenGaussMockData.buildAppEnvResponse());
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.eq(OpenGaussMockData.agentId2)))
            .thenReturn(Optional.of(OpenGaussMockData.getAgentEnvironment()));
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(OpenGaussMockData.getAgentEnvironmentLinkStatusIsOnline());
        openGaussEnvironmentProvider.register(clusterEnvironment);
        String uuid = clusterEnvironment.getUuid();
        Assert.assertNotNull(uuid);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), clusterEnvironment.getLinkStatus());
        Assert.assertEquals("24f7ecaf-d160-3f2f-a890-d0c244b6676c", uuid);
    }

    /**
     * 用例场景 openGauss集群环境注册
     * 前置条件：环境信息参数正确
     * 检查点: 集群重复注册 抛出 资源重复添加
     */
    @Test
    public void should_throw_LegoCheckedException_when_resource_repeat_register() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, clusterEnvironment))
            .thenReturn(resource);
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.eq(OpenGaussMockData.agentId)))
            .thenReturn(Optional.of(OpenGaussMockData.getAgentEnvironment()));
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.eq(OpenGaussMockData.agentId2)))
            .thenReturn(Optional.of(OpenGaussMockData.getAgentEnvironment2()));
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.eq("24f7ecaf-d160-3f2f-a890-d0c244b6676c")))
            .thenReturn(Optional.of(OpenGaussMockData.getAgentEnvironment()));
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.eq(OpenGaussMockData.environmentId)))
            .thenReturn(Optional.of(OpenGaussMockData.getAgentEnvironment2()));
        PowerMockito.mockStatic(ResourceCheckContextUtil.class);
        PowerMockito.mockStatic(OpenGaussClusterUtil.class);
        PowerMockito.when(OpenGaussClusterUtil.getContextClusterInfo(any()))
            .thenReturn(OpenGaussMockData.buildAppEnvResponse());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(OpenGaussMockData.getAgentEnvironmentLinkStatusIsOnline());

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> openGaussEnvironmentProvider.register(clusterEnvironment));

        Assert.assertEquals("The selected node has been registered.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景 openGauss集群环境注册
     * 前置条件：环境信息参数正确
     * 检查点: 集群注册dependencies中的agent已经处于离线状态，抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_environment_dependencies_agent_is_offline() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, clusterEnvironment))
            .thenReturn(resource);
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.of(OpenGaussMockData.getAgentEnvironment3LinkStatusIsOffline()));
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(OpenGaussMockData.getAgentEnvironment3LinkStatusIsOffline());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> openGaussEnvironmentProvider.register(clusterEnvironment));
        Assert.assertEquals(CommonErrorCode.HOST_OFFLINE, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景 openGauss集群环境注册
     * 前置条件：环境信息参数正确
     * 检查点: 查询集群信息endpoint为空，返回系统错误
     */
    @Test
    public void should_throw_LegoCheckedException_when_endpoint_of_cluster_is_null() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, clusterEnvironment))
            .thenReturn(resource);
        PowerMockito.mockStatic(ResourceCheckContextUtil.class);
        PowerMockito.mockStatic(OpenGaussClusterUtil.class);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(OpenGaussClusterUtil.getContextClusterInfo(any()))
            .thenReturn(OpenGaussMockData.buildAppEnvResponseNodeEndpointEmpty());
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(OpenGaussMockData.getAgentEnvironmentLinkStatusIsOnline());
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        Assert.assertThrows(LegoCheckedException.class, () -> openGaussEnvironmentProvider.register(clusterEnvironment));
    }

    private ProtectedEnvironment getClusterEnvironmentOfflineAgentEnv() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenGaussConstants.CLUSTER_STATE, OpenGaussClusterStateEnum.NORMAL.getState());
        extendInfo.put(OpenGaussConstants.NODES,
            JSONObject.writeValueAsString(Collections.singletonList(OpenGaussMockData.getNodeInfo())));
        clusterEnvironment.setExtendInfo(extendInfo);

        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedEnvironment offlineAgentEnv = getOfflineAgentEnv();
        dependency.put(DatabaseConstants.AGENTS, Collections.singletonList(offlineAgentEnv));
        clusterEnvironment.setDependencies(dependency);
        return clusterEnvironment;
    }

    private ProtectedEnvironment getOfflineAgentEnv() {
        ProtectedEnvironment offlineAgent = new ProtectedEnvironment();
        offlineAgent.setUuid(OpenGaussMockData.agentId3);
        offlineAgent.setEndpoint("8.3.6.7");
        offlineAgent.setPort(9636);
        offlineAgent.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        return offlineAgent;
    }

    private ProtectedEnvironment getClusterEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Authentication auth = new Authentication();
        auth.setAuthKey("omm");
        auth.setAuthType(Authentication.OS_PASSWORD);
        protectedEnvironment.setAuth(auth);
        protectedEnvironment.setDependencies(getDependency());
        protectedEnvironment.setName("TestClusterName");
        protectedEnvironment.setType(ResourceTypeEnum.DATABASE.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        protectedEnvironment.setEndpoint("8.3.6.63");
        return protectedEnvironment;
    }

    private Map<String, List<ProtectedResource>> getDependency() {
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedEnvironment agent2 = getProtectedEnvironment();
        dependency.put(DatabaseConstants.AGENTS, Collections.singletonList(agent2));
        return dependency;
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setUuid(OpenGaussMockData.agentId2);
        agent2.setEndpoint("8.3.6.6");
        agent2.setPort(9636);
        agent2.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        return agent2;
    }

    /**
     * 用例场景 注册的用户只能以 中文、字母和_开始
     * 前置条件：环境信息参数正确
     * 检查点: 注册的用户只能以 中文、字母和_开始 中文、字母和_开始校验通过，不是则返回false
     */
    @Test
    public void should_throw_LegoCheckedException_when_json_schema_validator() {
        Pattern compile = Pattern.compile("^[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$");
        // 非中文、字母和_开始校验失败
        Assert.assertFalse(compile.matcher("1dd11sdsd-").find());
        Assert.assertTrue(compile.matcher("_中dd11sdsd-").find());
        Assert.assertTrue(compile.matcher("中dd11sdsd-_查").find());
        Assert.assertTrue(compile.matcher("dd11sdsd-_查").find());
    }
}
