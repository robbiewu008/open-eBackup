/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.sqlserver.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.common.SqlServerErrorCode;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * sqlserver集群实例测试类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-19
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {EnvironmentLinkStatusHelper.class})
public class SqlServerClusterInstanceProviderTest {
    private ResourceService resourceService;

    private SqlServerClusterInstanceProvider sqlServerClusterInstanceProvider;

    private SqlServerBaseService sqlServerBaseService;

    private AgentUnifiedService agentUnifiedService;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
        sqlServerBaseService = Mockito.mock(SqlServerBaseService.class);
        agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        this.sqlServerClusterInstanceProvider = new SqlServerClusterInstanceProvider(agentUnifiedService,
            sqlServerBaseService);
    }

    /**
     * 用例场景：sqlserver集群类型检查类provider过滤
     * 前置条件：无
     * 检查点：集群实例类型类过滤成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(sqlServerClusterInstanceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：创建集群实例条件检查
     * 前置条件：无
     * 检查点：集群实例条件检查成功
     */
    @Test
    public void beforeCreate_success() {
        ProtectedResource protectedResource = getProtectedResource();
        AgentBaseDto agentDto = new AgentBaseDto();
        agentDto.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        JSONObject msg = new JSONObject();
        msg.put(DatabaseConstants.NAME, "");
        msg.put(DatabaseConstants.ROLE, 0);
        msg.put("state", true);
        agentDto.setErrorMessage(msg.toString());
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentDto).thenReturn(agentDto);
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        PowerMockito.when(sqlServerBaseService.getProtectedEnvironmentByResourceList(any()))
            .thenReturn(Collections.singletonList(protectedEnvironment));
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        sqlServerClusterInstanceProvider.beforeCreate(protectedResource);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：资源删除前的处理
     * 前置条件：无
     * 检查点：资源删除前的处理成功
     */
    @Test
    public void pre_handle_delete_success() {
        ProtectedResource agGroup = new ProtectedResource();
        agGroup.setUuid("alwaysOnUuid");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setUuid("clusterUuid");
        dependencies.put(SqlServerConstants.INSTANCE, Collections.singletonList(clusterInstance));
        agGroup.setDependencies(dependencies);
        Mockito.when(sqlServerBaseService.getResourceOfClusterByType(any(), any(), anyBoolean()))
            .thenReturn(Collections.singletonList(agGroup));
        ResourceDeleteContext resourceDeleteContext = sqlServerClusterInstanceProvider.preHandleDelete(clusterInstance);
        Assert.assertEquals(resourceDeleteContext.getResourceDeleteDependencyList().size(), 2);
    }

    /**
     * 用例场景：资源删除前的处理
     * 前置条件：无
     * 检查点：资源删除前的处理成功
     */
    @Test
    public void check_node_online_success() {
        ProtectedEnvironment host = new ProtectedEnvironment();
        host.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        List<ProtectedEnvironment> hosts = new ArrayList<ProtectedEnvironment>() {{
            add(host);
        }};
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        Assert.assertThrows("[SQL Server] cluster agent is offLine!", LegoCheckedException.class,
            () -> Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "checkNodeOnline", hosts));
    }

    /**
     * 用例场景：更新集群实例前置检查
     * 前置条件：无
     * 检查点：集群实例更新前置检查成功
     */
    @Test
    public void before_update_success() {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedEnvironment nodeOne = new ProtectedEnvironment();
        nodeOne.setUuid("1");
        nodeOne.setEndpoint("127.0.0.1");
        nodeOne.setPort(1433);
        agents.add(nodeOne);
        ProtectedEnvironment nodeTwo = new ProtectedEnvironment();
        nodeTwo.setUuid("2");
        nodeTwo.setEndpoint("127.0.0.1");
        nodeTwo.setPort(1433);
        agents.add(nodeTwo);
        dependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setParentUuid("clusterUuid");
        protectedResource.setUuid("0");
        protectedResource.setName("testClusterInstance");
        protectedResource.setDependencies(dependencies);
        protectedResource.setExtendInfo(new HashMap<>());
        Mockito.when(sqlServerBaseService.getResourceOfClusterByType(any(), any(), anyBoolean()))
            .thenReturn(new ArrayList<>());
        AgentBaseDto agentDto = new AgentBaseDto();
        agentDto.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        JSONObject msg = new JSONObject();
        msg.put(DatabaseConstants.NAME, "");
        msg.put(DatabaseConstants.ROLE, 0);
        msg.put("state", true);
        agentDto.setErrorMessage(msg.toString());
        List<ProtectedEnvironment> hosts = new ArrayList<>();
        hosts.add(nodeOne);
        hosts.add(nodeTwo);
        Mockito.when(sqlServerBaseService.getProtectedEnvironmentByResourceList(any()))
            .thenReturn(Collections.singletonList(nodeOne));
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentDto).thenReturn(agentDto);
        Mockito.when(sqlServerBaseService.getResourceByUuid(any())).thenReturn(protectedResource);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        sqlServerClusterInstanceProvider.beforeUpdate(protectedResource);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：更新集群实例前置检查
     * 前置条件：当要更新的节点已经被其他实例所注册过
     * 检查点：集群实例更新前置检查将抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_before_update_with_wrong_param() {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedResource nodeOne = new ProtectedResource();
        nodeOne.setUuid("1");
        agents.add(nodeOne);
        dependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setParentUuid("clusterUuid");
        clusterInstance.setUuid("0");
        clusterInstance.setName("testClusterInstance");
        clusterInstance.setDependencies(dependencies);
        clusterInstance.setExtendInfo(new HashMap<>());
        ProtectedResource copy = BeanTools.copy(clusterInstance, ProtectedResource::new);
        copy.setParentUuid("clusterUuidOld");
        Mockito.when(sqlServerBaseService.getResourceByUuid(any())).thenReturn(copy);
        Assert.assertThrows("[SQL Server Cluster Update] cluster instance parent can not change.",
            LegoCheckedException.class, () -> sqlServerClusterInstanceProvider.beforeUpdate(clusterInstance));
        ProtectedResource nodeTwo = new ProtectedResource();
        nodeTwo.setUuid("2");
        List<ProtectedResource> agentsOld = new ArrayList<>();
        agentsOld.add(nodeTwo);
        agentsOld.add(nodeOne);
        Map<String, List<ProtectedResource>> dependenciesOld = new HashMap<>();
        dependenciesOld.put(DatabaseConstants.AGENTS, agentsOld);
        copy.setDependencies(dependenciesOld);
        copy.setParentUuid("clusterUuid");
        Assert.assertThrows(Exception.class,
            () -> sqlServerClusterInstanceProvider.beforeUpdate(clusterInstance));
    }

    /**
     * 用例场景：创建集群实例前置检查
     * 前置条件：集群注册无依赖
     * 检查点：集群条件检查失败抛出参数异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_instance_has_no_dependencies_when_beforeCreate() {
        ProtectedResource protectedResource = getProtectedResource();
        protectedResource.setDependencies(null);
        Assert.assertThrows("[SQL Server] cluster nodes not found", LegoCheckedException.class,
            () -> sqlServerClusterInstanceProvider.beforeCreate(protectedResource));
    }

    /**
     * 用例场景：创建集群实例前置检查
     * 前置条件：集群注册正常且已经注册
     * 检查点：集群条件检查失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_node_registered_when_beforeCreate() {
        ProtectedResource protectedResource = getProtectedResource();
        PowerMockito.when(sqlServerBaseService.getResourceOfClusterByType(any(), any(), anyBoolean()))
            .thenReturn(Collections.singletonList(protectedResource));
        Assert.assertThrows("[SQL Server] cluster nodes has registered", LegoCheckedException.class,
            () -> sqlServerClusterInstanceProvider.beforeCreate(protectedResource));
    }

    /**
     * 用例场景：创建集群实例前置检查
     * 前置条件：集群实例节点数量大于最大节点数
     * 检查点：集群条件检查失败抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_node_num_error_when_beforeCreate() {
        ProtectedResource protectedResource = getProtectedResource();
        ProtectedResource node1 = new ProtectedResource();
        ProtectedResource node2 = new ProtectedResource();
        protectedResource.getDependencies().get(DatabaseConstants.AGENTS).add(node1);
        protectedResource.getDependencies().get(DatabaseConstants.AGENTS).add(node2);
        Assert.assertThrows("[SQL Server] cluster nodes num error", LegoCheckedException.class,
            () -> sqlServerClusterInstanceProvider.beforeCreate(protectedResource));
    }

    /**
     * 用例场景：集群实例的资源配置检查
     * 前置条件：无
     * 检查点：集群实例的资源配置检查成功
     */
    @Test
    public void get_resource_feature_success() {
        Assert.assertFalse(sqlServerClusterInstanceProvider.getResourceFeature().isShouldCheckResourceNameDuplicate());
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1234567890");
        protectedResource.setParentUuid("1234567890");
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource resourceItem = new ProtectedResource();
        resourceItem.setParentUuid("11111");
        resourceItem.setUuid("1234567890");
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(resourceItem);
        dependency.put(DatabaseConstants.AGENTS, resources);
        protectedResource.setDependencies(dependency);
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        Map<String, String> ext = new HashMap<>();
        ext.put(DatabaseConstants.INSTANCE_PORT, "3306");
        ext.put(DatabaseConstants.HOST_ID, "host-uuid");
        Authentication auth = new Authentication();
        auth.setAuthKey("root");
        auth.setAuthPwd("123456");
        auth.setExtendInfo(ext);
        protectedResource.setAuth(auth);
        resourceItem.setExtendInfo(ext);
        Map<String, List<ProtectedResource>> itemDependency = new HashMap<>();
        ProtectedResource deProtectedResource = new ProtectedResource();
        deProtectedResource.setUuid("host-id-123456");
        itemDependency.put(DatabaseConstants.AGENTS, Collections.singletonList(deProtectedResource));
        resourceItem.setDependencies(itemDependency);
        // 构建集群实例环境信息
        ProtectedEnvironment env = new ProtectedEnvironment();
        ProtectedEnvironment envDependency = new ProtectedEnvironment();
        envDependency.setEndpoint("127.0.0.1");
        envDependency.setPort(100);
        envDependency.setUuid("host-uuid");
        env.setUuid("host-uuid");
        HashMap<String, List<ProtectedResource>> dens = new HashMap<>();
        dens.put(DatabaseConstants.AGENTS, Collections.singletonList(envDependency));
        env.setDependencies(dens);
        protectedResource.setEnvironment(env);
        protectedResource.setExtendInfo(new HashMap<>());
        return protectedResource;
    }

    /**
     * 用例场景：sqlserver集群实例条件检查失败
     * 前置条件：集群注册正常且已经注册
     * 检查点：集群条件检查失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_instance_exist_when_beforeCreate() {
        ProtectedResource protectedResource = getProtectedResource();
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>();
        protectedResourcePageListResponse.setRecords(Collections.singletonList(new ProtectedResource()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any()))
            .thenReturn(protectedResourcePageListResponse);
        Assert.assertThrows(IndexOutOfBoundsException.class,
            () -> sqlServerClusterInstanceProvider.beforeCreate(protectedResource));
    }

    /**
     * 用例场景：sqlserver集群实例条件检查失败
     * 前置条件：集群注册正常，集群下有两个节点，而仅注册了一个集群实例节点
     * 检查点：集群条件检查失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_not_cluster_when_beforeCreate() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource protectedResource = getProtectedResource();
        AgentBaseDto agentDto = new AgentBaseDto();
        agentDto.setErrorCode(String.valueOf(CommonErrorCode.OPERATION_FAILED));
        ActionResult actionResult = new ActionResult();
        actionResult.setBodyErr(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        actionResult.setCode(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS);
        JSONObject msg = new JSONObject();
        msg.put(DatabaseConstants.NAME, "");
        msg.put(DatabaseConstants.ROLE, 0);
        actionResult.setMessage(msg.toString());
        agentDto.setErrorMessage(JSONObject.fromObject(actionResult).toString());
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentDto);
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        PowerMockito.when(sqlServerBaseService.getProtectedEnvironmentByResourceList(any()))
            .thenReturn(Collections.singletonList(protectedEnvironment));
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        sqlServerClusterInstanceProvider.beforeCreate(protectedResource);
    }

    /**
     * 用例场景：为集群实例添加主节点扩展信息
     * 前置条件：Agent返回信息正确
     * 检查点：为集群实例添加主节点扩展信息成功
     */
    @Test
    public void add_master_node_for_cluster_node_success() throws Exception {
        AgentBaseDto nodeOneDto = new AgentBaseDto();
        nodeOneDto.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        JSONObject msgOne = new JSONObject();
        msgOne.put(DatabaseConstants.ROLE, 1);
        nodeOneDto.setErrorMessage(msgOne.toString());

        AgentBaseDto nodeTwoDto = new AgentBaseDto();
        nodeTwoDto.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        JSONObject msgTwo = new JSONObject();
        msgTwo.put(DatabaseConstants.ROLE, 1);
        nodeTwoDto.setErrorMessage(msgTwo.toString());

        List<AgentBaseDto> checkResultList = new ArrayList<>();
        checkResultList.add(nodeOneDto);
        checkResultList.add(nodeTwoDto);
        List<ProtectedEnvironment> nodes = new ArrayList<>();
        ProtectedEnvironment nodeOne = new ProtectedEnvironment();
        ProtectedEnvironment nodeTwo = new ProtectedEnvironment();
        nodeOne.setUuid("1");
        nodeTwo.setUuid("2");
        nodes.add(nodeOne);
        nodes.add(nodeTwo);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(new HashMap<>());
        Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "addMasterNodeForClusterNode", protectedResource,
            checkResultList, nodes);
        Assert.assertEquals(protectedResource.getExtendInfo().get(SqlServerConstants.MASTER_NODE), "1");

        JSONObject msgOneNotMaster = new JSONObject();
        msgOneNotMaster.put(DatabaseConstants.ROLE, 0);
        nodeOneDto.setErrorMessage(msgOneNotMaster.toString());
        Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "addMasterNodeForClusterNode", protectedResource,
            checkResultList, nodes);
        Assert.assertEquals(protectedResource.getExtendInfo().get(SqlServerConstants.MASTER_NODE), "2");
    }

    /**
     * 用例场景：通过Agent返回值判断节点是否属于同一集群
     * 前置条件：Agent返回信息正确
     * 检查点：获判断节点是否属于同一集群成功
     */
    @Test
    public void analysis_cluster_instance_by_check_result_success() throws Exception {
        AgentBaseDto nodeOneDto = new AgentBaseDto();
        nodeOneDto.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        JSONObject msgOne = new JSONObject();
        msgOne.put(DatabaseConstants.NAME, "cluster-1");
        msgOne.put("state", false);
        nodeOneDto.setErrorMessage(msgOne.toString());

        List<AgentBaseDto> checkResultList = new ArrayList<>();
        checkResultList.add(nodeOneDto);
        Object getValueOfCheckResultByKeyOne = Whitebox.invokeMethod(sqlServerClusterInstanceProvider,
            "analysisClusterInstanceByCheckResult", checkResultList);
        Assert.assertEquals(false, getValueOfCheckResultByKeyOne);

        AgentBaseDto nodeTwoDto = new AgentBaseDto();
        nodeTwoDto.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        JSONObject msgTwo = new JSONObject();
        msgTwo.put(DatabaseConstants.NAME, "cluster-1");
        msgTwo.put("state", true);
        nodeTwoDto.setErrorMessage(msgTwo.toString());
        checkResultList.add(nodeTwoDto);
        Object getValueOfCheckResultByKeyTwo = Whitebox.invokeMethod(sqlServerClusterInstanceProvider,
            "analysisClusterInstanceByCheckResult", checkResultList);
        Assert.assertEquals(true, getValueOfCheckResultByKeyTwo);

        checkResultList.add(nodeTwoDto);
        Object getValueOfCheckResultByKeyThree = Whitebox.invokeMethod(sqlServerClusterInstanceProvider,
            "analysisClusterInstanceByCheckResult", checkResultList);
        Assert.assertEquals(false, getValueOfCheckResultByKeyThree);
    }

    /**
     * 用例场景：通过Agent返回值获取当前节点角色信息
     * 前置条件：Agent返回错误码
     * 检查点：获取当前节点角色信息失败抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_value_of_check_result_by_key_fail() {
        AgentBaseDto result = new AgentBaseDto();
        result.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        Assert.assertThrows("get virtualServiceName from agent fail", LegoCheckedException.class,
            () -> Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "getValueOfCheckResultByKey", result,
                DatabaseConstants.ROLE));
    }

    /**
     * 用例场景：通过Agent返回值获取当前节点角色信息
     * 前置条件：Agent返回errorCode不为0
     * 检查点：获取当前节点角色信息失败抛出操作失败异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_value_of_check_result_by_key_with_error_code_return() {
        AgentBaseDto result = new AgentBaseDto();
        result.setErrorCode(String.valueOf(SqlServerErrorCode.SQLSERVER_RESTORE_INSTANCE_INSUFFICIENT));
        Assert.assertThrows("get virtualServiceName from agent fail", LegoCheckedException.class,
            () -> Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "getValueOfCheckResultByKey", result,
                DatabaseConstants.ROLE));
    }

    /**
     * 用例场景：通过Agent返回值获取当前节点角色信息
     * 前置条件：Agent返回有errorMessage，但无对应的key值
     * 检查点：获取当前节点角色信息失败抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_value_of_check_result_by_key_with_no_key_value() {
        AgentBaseDto result = new AgentBaseDto();
        result.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        ActionResult actionResult = new ActionResult();
        actionResult.setMessage(new JSONObject().toString());
        result.setErrorMessage(JSONObject.fromObject(actionResult).toString());
        Assert.assertThrows("node not belong to cluster instance", LegoCheckedException.class,
            () -> Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "getValueOfCheckResultByKey", result,
                DatabaseConstants.ROLE));
    }

    /**
     * 用例场景：通过Agent返回值获取当前节点角色信息
     * 前置条件：Agent返回errorMessage中找不到对应的Key
     * 检查点：获取当前节点角色信息失败抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_value_of_check_result_by_key_with_no_key() {
        AgentBaseDto result = new AgentBaseDto();
        result.setErrorCode(String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS));
        JSONObject message = new JSONObject();
        result.setErrorMessage(message.toString());
        Assert.assertThrows("get virtualServiceName from agent fail", LegoCheckedException.class,
            () -> Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "getValueOfCheckResultByKey", result,
                DatabaseConstants.ROLE));
    }

    /**
     * 用例场景：通过Agent校验节点
     * 前置条件：Agent返回errorCode不为0，同时errorMessage中bodyErr字段值为空
     * 检查点：集群节点Agent校验失败抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_check_result_by_agent_error_without_bodyErr() {
        AgentBaseDto result = new AgentBaseDto();
        result.setErrorCode(String.valueOf(SqlServerErrorCode.CHECK_CLUSTER_NUM_FAILED));
        JSONObject message = new JSONObject();
        result.setErrorMessage(message.toString());
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(result);
        Assert.assertThrows("Cluster instance check by agent return code.", LegoCheckedException.class,
            () -> Whitebox.invokeMethod(sqlServerClusterInstanceProvider, "getCheckResultByAgent",
                getProtectedResource(), getProtectedEnvironment()));
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3388);
        protectedEnvironment.setName("MY-PC");
        Map<String, String> ext = new HashMap<>();
        ext.put(DatabaseConstants.HOST_ID, "host-uuid");
        protectedEnvironment.setName("MY-PC");
        protectedEnvironment.setExtendInfo(ext);
        return protectedEnvironment;
    }
}