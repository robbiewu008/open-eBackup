package openbackup.mongodb.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mongodb.protection.access.bo.MongoClusterNodesExtendInfo;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.enums.MongoDBClusterRoleEnum;
import openbackup.mongodb.protection.access.enums.MongoDBClusterTypeEnum;
import openbackup.mongodb.protection.access.enums.MongoDBNodeTypeEnum;
import openbackup.mongodb.protection.access.mock.MongoDBMockBean;

import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.mongodb.protection.access.service.impl.MongoDBBaseServiceImpl;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * MongoDB集群provider 测试类
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class MongoDBClusterProviderTest {
    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);

    private final JsonSchemaValidator jsonSchemaValidator = Mockito.mock(JsonSchemaValidator.class);

    private final MongoDBBaseService mongoDBBaseService = Mockito.mock(MongoDBBaseService.class);

    private final MongoDBMockBean mongoDBMockBean = new MongoDBMockBean();

    private final MongoDBClusterProvider mongoDBClusterProvider = new MongoDBClusterProvider(providerManager,
        pluginConfigManager, jsonSchemaValidator, mongoDBBaseService);

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(
        ProtectedEnvironmentService.class);

    private final MongoDBBaseService mongoDBBaseService1 = new MongoDBBaseServiceImpl(resourceService,
        agentUnifiedService, protectedEnvironmentService);

    private final MongoDBClusterProvider mongoDBClusterProvider1 = new MongoDBClusterProvider(providerManager,
        pluginConfigManager, jsonSchemaValidator, mongoDBBaseService1);

    /**
     * 用例场景：MongoDB集群注册下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mongoDBClusterProvider.applicable(ResourceSubTypeEnum.MONGODB_CLUSTER.getType()));
    }

    /**
     * 用例场景：MongoDB集群注册成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_success() {
        ProtectedEnvironment mongoDBProtectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        mongoDBProtectedEnvironment.setUuid("uuid");
        mongoDBProtectedEnvironment.setExtendInfo(new HashMap<>());
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfoByKey(MongoDBConstants.SERVICE_IP, "8.40.96.214");
        protectedResource.setExtendInfoByKey(MongoDBConstants.SERVICE_PORT, "28017");
        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.APP_PASSWORD);
        authentication.setAuthKey("root");
        protectedResource.setAuth(authentication);
        protectedResources.add(protectedResource);
        protectedResources.add(protectedResource);
        given(mongoDBBaseService.getResource(any())).willReturn(protectedResource);
        AppEnvResponse mongoDBAppEnvResponse = mongoDBMockBean.getMongoDBAppEnvResponse();
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.EXIST_NODES, "1");
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.AGENT_HOST, "8.40.96.214:28017");
        List<NodeInfo> nodeInfos = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setName("8.40.96.214:28017");
        nodeInfos.add(nodeInfo);
        nodeInfos.add(nodeInfo);
        mongoDBAppEnvResponse.setNodes(nodeInfos);
        given(mongoDBBaseService.getAppEnvAgentInfo(any(), any())).willReturn(mongoDBAppEnvResponse);
        given(mongoDBBaseService.getEnvironmentById(any())).willReturn(mongoDBProtectedEnvironment);
        List<String> list = new ArrayList<>();
        List<AppEnvResponse> list1 = new ArrayList<>();
        list1.add(mongoDBAppEnvResponse);
        list1.add(mongoDBAppEnvResponse);
        list.add("8.40.96.214:28017");
        given(mongoDBBaseService.getAllIpAndPortList(any())).willReturn(list);
        given(
            mongoDBBaseService.getAppEnvResponses(any(ProtectedResource.class), any(), any(Boolean.class))).willReturn(
            list1);
        mongoDBProtectedEnvironment.getDependencies().put(DatabaseConstants.CHILDREN, protectedResources);
        mongoDBClusterProvider.register(mongoDBProtectedEnvironment);
        Assert.assertTrue(true);
    }

    private ProtectedEnvironment getMongoDBProtectedEnvironment() {
        ProtectedEnvironment mongoDBProtectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        mongoDBProtectedEnvironment.setUuid("uuid");
        mongoDBProtectedEnvironment.setExtendInfo(new HashMap<>());
        mongoDBProtectedEnvironment.getExtendInfo()
            .put(MongoDBConstants.CLUSTE_TYPE, MongoDBClusterTypeEnum.SHARD.getType());
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfoByKey(MongoDBConstants.SERVICE_IP, "8.40.96.214");
        protectedResource.setExtendInfoByKey(MongoDBConstants.SERVICE_PORT, "28017");
        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.APP_PASSWORD);
        authentication.setAuthKey("root");
        authentication.setAuthPwd("root");
        protectedResource.setAuth(authentication);
        protectedResources.add(protectedResource);
        protectedResources.add(protectedResource);
        given(mongoDBBaseService.getResource(any())).willReturn(protectedResource);
        AppEnvResponse mongoDBAppEnvResponse = mongoDBMockBean.getMongoDBAppEnvResponse();
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.EXIST_NODES, "1");
        mongoDBAppEnvResponse.getExtendInfo()
            .put(MongoDBConstants.SHARD_CLUSTER_TYPE, MongoDBNodeTypeEnum.MONGOS.getType());
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.AGENT_HOST, "8.40.96.214:28017");
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.LOCAL_HOST, "8.40.96.214:28017");
        mongoDBAppEnvResponse.getExtendInfo()
            .put(MongoDBConstants.AGENT_SHARD_LIST,
                "sh1/8.40.96.211:27018,8.40.96.211:27019,8.40.96.211:27020;sh2/8.40.96.212:27018,8.40.96.212:27019,8.40.96.212:27020;sh3/8.40.96.213:27018,8.40.96.213:27019,8.40.96.213:27020");
        AppEnvResponse mongoDBAppEnvResponse1 = mongoDBMockBean.getMongoDBAppEnvResponse();
        mongoDBAppEnvResponse1.getExtendInfo().put(MongoDBConstants.EXIST_NODES, "1");
        mongoDBAppEnvResponse1.getExtendInfo()
            .put(MongoDBConstants.SHARD_CLUSTER_TYPE, MongoDBNodeTypeEnum.SHARD.getType());
        mongoDBAppEnvResponse1.getExtendInfo().put(MongoDBConstants.AGENT_HOST, "8.40.96.214:28018");
        mongoDBAppEnvResponse1.getExtendInfo().put(MongoDBConstants.LOCAL_HOST, "8.40.96.214:28017");
        mongoDBAppEnvResponse1.getExtendInfo().put(MongoDBConstants.AGENT_NODES, "8.40.96.214:28017");
        List<NodeInfo> nodeInfos1 = new ArrayList<>();
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setName("8.40.96.214:28017");
        nodeInfos1.add(nodeInfo1);
        nodeInfos1.add(nodeInfo1);
        mongoDBAppEnvResponse1.setNodes(nodeInfos1);
        List<NodeInfo> nodeInfos = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setName("8.40.96.214:28017");
        nodeInfos.add(nodeInfo);
        nodeInfos.add(nodeInfo);
        mongoDBAppEnvResponse.setNodes(nodeInfos);
        given(mongoDBBaseService.getAppEnvAgentInfo(any(), any())).willReturn(mongoDBAppEnvResponse);
        given(mongoDBBaseService.getEnvironmentById(any())).willReturn(mongoDBProtectedEnvironment);
        List<String> list = new ArrayList<>();
        List<AppEnvResponse> list1 = new ArrayList<>();
        list1.add(mongoDBAppEnvResponse);
        list1.add(mongoDBAppEnvResponse1);
        list.add("8.40.96.214:28017");
        list.add("8.40.96.214:28018");
        given(mongoDBBaseService.getAllIpAndPortList(any())).willReturn(list);
        given(
            mongoDBBaseService.getAppEnvResponses(any(ProtectedResource.class), any(), any(Boolean.class))).willReturn(
            list1);
        mongoDBProtectedEnvironment.getDependencies().put(DatabaseConstants.CHILDREN, protectedResources);
        return mongoDBProtectedEnvironment;
    }

    /**
     * 用例场景：MongoDB集群注册成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_11_success() {
        mongoDBClusterProvider.register(getMongoDBProtectedEnvironment());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：MongoDB集群children数量少于两个
     * 前置条件：无
     * 检查点：children数量少于两个
     */
    @Test
    public void should_throw_LegoCheckedException_if_children_size_low_two_exist_when_check() {
        ProtectedEnvironment mongoDBProtectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        mongoDBProtectedEnvironment.setUuid("uuid");
        Assert.assertThrows(LegoCheckedException.class,
            () -> mongoDBClusterProvider.register(mongoDBProtectedEnvironment));
    }

    /**
     * 用例场景：MongoDB集群健康检查成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment mongoDBProtectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        mongoDBProtectedEnvironment.setUuid("uuid");
        List<MongoClusterNodesExtendInfo> list = new ArrayList<>();
        MongoClusterNodesExtendInfo nodesExtendInfo = new MongoClusterNodesExtendInfo();
        nodesExtendInfo.setStateStr(MongoDBClusterRoleEnum.PRIMARY.getRole());
        nodesExtendInfo.setHostUrl("8.40.96.214:28017");
        MongoClusterNodesExtendInfo nodesExtendInfo1 = new MongoClusterNodesExtendInfo();
        nodesExtendInfo.setStateStr(MongoDBClusterRoleEnum.SECONDARY.getRole());
        nodesExtendInfo.setHostUrl("8.40.96.214:28017");
        list.add(nodesExtendInfo);
        list.add(nodesExtendInfo1);
        mongoDBProtectedEnvironment.setExtendInfoByKey(MongoDBConstants.CLUSTER_NODES, JSONArray.fromObject(list).toString());
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfoByKey(MongoDBConstants.SERVICE_IP, "8.40.96.214");
        protectedResource.setExtendInfoByKey(MongoDBConstants.SERVICE_PORT, "28017");
        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.APP_PASSWORD);
        authentication.setAuthKey("root");
        protectedResource.setAuth(authentication);
        protectedResources.add(protectedResource);
        protectedResources.add(protectedResource);
        given(resourceService.getResourceById(any())).willReturn(Optional.of(protectedResource));
        AppEnvResponse mongoDBAppEnvResponse = mongoDBMockBean.getMongoDBAppEnvResponse();
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.EXIST_NODES, "1");
        mongoDBAppEnvResponse.getExtendInfo()
            .put(MongoDBConstants.SHARD_CLUSTER_TYPE, MongoDBNodeTypeEnum.REPLICATION.getType());
        List<NodeInfo> nodeInfos = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setName("8.40.96.214:28017");
        nodeInfo.setExtendInfo(new HashMap<>());
        nodeInfo.getExtendInfo().put(MongoDBConstants.STATE_STR, MongoDBClusterRoleEnum.SECONDARY.getRole());
        nodeInfos.add(nodeInfo);
        nodeInfos.add(nodeInfo);
        nodeInfos.add(nodeInfo);
        mongoDBAppEnvResponse.setNodes(nodeInfos);
        given(agentUnifiedService.getClusterInfo(any(), any())).willReturn(mongoDBAppEnvResponse);
        given(protectedEnvironmentService.getEnvironmentById(any())).willReturn(mongoDBProtectedEnvironment);
        mongoDBProtectedEnvironment.getDependencies().put(DatabaseConstants.CHILDREN, protectedResources);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        Assert.assertThrows(LegoCheckedException.class, () -> mongoDBClusterProvider1.validate(mongoDBProtectedEnvironment));
        nodeInfo.getExtendInfo().put(MongoDBConstants.STATE_STR, MongoDBClusterRoleEnum.PRIMARY.getRole());
        nodeInfos.add(nodeInfo);
        nodeInfos.add(nodeInfo);
        nodeInfos.add(nodeInfo);
        mongoDBAppEnvResponse.setNodes(nodeInfos);
        given(agentUnifiedService.getClusterInfo(any(), any())).willReturn(mongoDBAppEnvResponse);
        mongoDBProtectedEnvironment.setExtendInfoByKey(DatabaseConstants.NODE_COUNT, "1");
        mongoDBClusterProvider1.validate(mongoDBProtectedEnvironment);
    }
}
