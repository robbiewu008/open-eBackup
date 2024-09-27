/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.oracle.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.constants.OracleConstants;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.dee.DeeDbAnonymizationRest;
import openbackup.system.base.service.DeployTypeService;

import junit.framework.TestCase;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

@RunWith(PowerMockRunner.class)
public class OracleBaseServiceTest extends TestCase {
    @Rule
    public final ExpectedException expectedException = ExpectedException.none();
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    private final DeeDbAnonymizationRest deeDbAnonymizationRest = Mockito.mock(DeeDbAnonymizationRest.class);
    private final DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);
    private final CommonAlarmService commonAlarmService = Mockito.mock(CommonAlarmService.class);

    private final OracleBaseService oracleBaseService =
        new OracleBaseService(resourceService, agentUnifiedService, deeDbAnonymizationRest, deployTypeService,
            commonAlarmService);

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. 存在一个Agent主机信息
     * 检 查 点：1. 获取到Agent主机信息和预期一样
     */
    @Test
    public void get_agent_by_single_instance_uuid_success() {
        String singleInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        List<ProtectedResource> agentResources = new ArrayList<>();
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        agentResources.add(agentEnv);
        dependencies.put("agents", agentResources);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
                .thenReturn(Optional.of(protectedResource));
        ProtectedEnvironment agentReturnEnv = oracleBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid);
        Assert.assertTrue(agentReturnEnv.equals(agentEnv));
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. Agent主机信息不是环境
     * 检 查 点：1. 报错，且错误信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_agent_when_agent_is_not_env() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("oracle agent resource is not env.");
        String singleInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        List<ProtectedResource> agentResources = new ArrayList<>();
        ProtectedResource agentRes = new ProtectedResource();
        agentRes.setUuid(UUID.randomUUID().toString());
        agentResources.add(agentRes);
        dependencies.put("agents", agentResources);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
                .thenReturn(Optional.of(protectedResource));
        oracleBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid);
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint
     * 前置条件：1. ip和port不为空
     * 检 查 点：1. 不报错且获取到endpoint信息和预期一样
     */
    @Test
    public void get_agent_endpoint_success() {
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setEndpoint("8.40.99.101");
        env.setPort(2181);
        env.setUuid("1111");
        env.setOsType(OracleConstants.WINDOWS);
        Endpoint agentEndpoint = oracleBaseService.getAgentEndpoint(env);
        Assert.assertTrue(agentEndpoint.getId().equals(env.getUuid()));
        Assert.assertTrue(agentEndpoint.getIp().equals(env.getEndpoint()));
        Assert.assertTrue(agentEndpoint.getPort() == env.getPort());
        Assert.assertTrue(agentEndpoint.getAgentOS().equalsIgnoreCase(env.getOsType()));
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint,且osType为空
     * 前置条件：1. osType不为windows
     * 检 查 点：1. 不报错且获取到endpoint信息和预期一样
     */
    @Test
    public void get_agent_endpoint_success_osType_is_null() {
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setEndpoint("8.40.99.101");
        env.setPort(2181);
        env.setUuid("1111");
        env.setOsType("linux");
        Endpoint agentEndpoint = oracleBaseService.getAgentEndpoint(env);
        Assert.assertTrue(agentEndpoint.getId().equals(env.getUuid()));
        Assert.assertTrue(agentEndpoint.getIp().equals(env.getEndpoint()));
        Assert.assertTrue(agentEndpoint.getPort() == env.getPort());
        Assert.assertNull(agentEndpoint.getAgentOS());
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint
     * 前置条件：1. uuid为空
     * 检 查 点：1. 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_uuid_is_null(){
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        oracleBaseService.getAgentEndpoint(environment);
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint
     * 前置条件：1. uuid为空
     * 检 查 点：1. 抛出异常
     */
    @Test
    public void test_get_environment_by_id_success(){
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("12340");
        Optional<ProtectedResource> optional = Optional.of(environment);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(optional);
        ProtectedEnvironment resource = oracleBaseService.getEnvironmentById("12340");
        Assert.assertTrue(resource.getUuid().equals(environment.getUuid()));
    }

    @Test
    public void test_get_single_instance_by_cluster_instance_success(){
        ProtectedResource resource = new ProtectedResource();
        resource.setDependencies(new HashMap<>());
        List<ProtectedResource> agents = new ArrayList<>();
        resource.getDependencies().put(DatabaseConstants.AGENTS, agents);
        Optional<ProtectedResource> optional = Optional.of(resource);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(optional);
        List<ProtectedResource> result = oracleBaseService.getSingleInstanceByClusterInstance("resource");
        Assert.assertNotNull(result);
    }

    /**
     * 用例场景：刷新子实例信息
     * 前置条件：1. 未获取到所有参数
     * 检 查 点：1. 抛出LegoCheckedException异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_not_contains_all_params() {
        expectedException.expect(DataProtectionAccessException.class);
        expectedException.expectMessage("get oracle cluster error.");
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(),any())).thenReturn(null);
        oracleBaseService.refreshClusterInstanceActiveStandby(new ProtectedResource(), new ProtectedEnvironment());
    }

    /**
     * 用例场景：刷新子实例信息
     * 前置条件：1. oracle未启动
     * 检 查 点：1. 抛出LegoCheckedException异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_oracle_not_running() {
        expectedException.expect(DataProtectionAccessException.class);
        expectedException.expectMessage("oracle is not running.");
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(mockExtendInfo());
        appEnvResponse.getExtendInfo().put("status","1");
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(),any())).thenReturn(appEnvResponse);
        oracleBaseService.refreshClusterInstanceActiveStandby(new ProtectedResource(), new ProtectedEnvironment());
    }

    /**
     * 用例场景：刷新子实例信息
     * 前置条件：mock
     * 检查点：刷新成功
     */
    @Test
    public void should_refresh_cluster_when_oracle_running() {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(mockExtendInfo());
        appEnvResponse.getExtendInfo().put("status","0");
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("resId");
        resource.setExtendInfo(new HashMap<>());
        AgentDetailDto detail = new AgentDetailDto();
        detail.setResourceList(Collections.singletonList(new AppResource()));

        PowerMockito.when(agentUnifiedService.getClusterInfo(any(),any())).thenReturn(appEnvResponse);
        PowerMockito.when(agentUnifiedService.getDetail(any(), any(), any(), any())).thenReturn(detail);

        oracleBaseService.refreshClusterInstanceActiveStandby(resource, new ProtectedEnvironment());
        Assert.assertEquals("is_asm_inst", resource.getExtendInfoByKey("is_asm_inst"));
    }

    /**
     * 用例场景：将单实例信息的auth设置到单实例对应的nodes的auth中
     * 前置条件：mock
     * 检查点：node的auth不为空，扩展信息中有oracle_home信息
     */
    @Test
    public void should_set_nodes_auth_successful() {
        List<TaskEnvironment> nodes = mockEnvList();
        List<ProtectedResource> singleInstanceResources = mockResourceList();
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(getOptionalEnv());

        oracleBaseService.setNodesAuth(nodes, singleInstanceResources);
        Assert.assertNotNull(nodes.get(0).getAuth());
        Assert.assertEquals("test oracle.", nodes.get(0).getExtendInfo().get("oracle_home"));
    }

    /**
     * 用例场景：更新主机asm信息
     * 前置条件：mock
     * 检查点：资源扩展字段中的asm不为空
     */
    @Test
    public void should_fill_asm_info_successful() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("resId");
        resource.setExtendInfo(new HashMap<>());
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("envId");
        AgentDetailDto detail = new AgentDetailDto();
        detail.setResourceList(Collections.singletonList(new AppResource()));
        PowerMockito.when(agentUnifiedService.getDetail(any(), any(), any(), any())).thenReturn(detail);

        oracleBaseService.fillAsmInfo(resource, environment);
        Assert.assertEquals("asm", resource.getExtendInfoByKey("queryType"));
    }

    /**
     * 用例场景：设置恢复node
     * 前置条件：mock
     * 检查点：设置恢复node
     */
    @Test
    public void should_supply_node() {
        // given
        RestoreTask restoreTask = new RestoreTask();
        List<Endpoint> agents = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("1");
        endpoint.setIp("10.0.0.1");
        endpoint.setPort(22);
        agents.add(endpoint);
        restoreTask.setAgents(agents);
        restoreTask.setTargetEnv(new TaskEnvironment());
        HostDto hostDto = new HostDto();
        Map<String, String> map = new HashMap<>();
        map.put("key", "value");
        hostDto.setExtendInfo(JSONObject.stringify(map));
        PowerMockito.when(agentUnifiedService.getHost(any(), any())).thenReturn(hostDto);
        // when
        oracleBaseService.supplyNodes(restoreTask);
        // then
        Assert.assertEquals("value", restoreTask.getTargetEnv().getNodes().get(0).getExtendInfo().get("key"));
    }

    /**
     * 用例场景：查询oracle数据库是否可以删除
     * 前置条件：dee接口返回未有任务执行
     * 检查点：返回可以删除
     */
    @Test
    public void query_resource_deletable_success() {
        PowerMockito.when(deeDbAnonymizationRest.isAnonymizationRunning(anyString())).thenReturn(false);
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X6000);
        Assert.assertTrue(oracleBaseService.isAnonymizationDeletable("123"));
    }

    @Test
    public void should_repository_adapts_windows_success() {
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository dataStorageRepository = new StorageRepository();
        dataStorageRepository.setType(RepositoryTypeEnum.DATA.getType());
        dataStorageRepository.setExtendInfo(null);
        repositories.add(dataStorageRepository);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setOsType("windows");
        oracleBaseService.repositoryAdaptsWindows(repositories, environment);
        Assert.assertEquals(RepositoryProtocolEnum.CIFS.getProtocol(),
                (int) repositories.get(0).getProtocol());
        Assert.assertEquals(repositories.get(0).getExtendInfo()
                .get("everyoneAuth").toString(), "true");
    }

    private Map<String,String> mockExtendInfo(){
        Map<String,String> extendInfo=new HashMap<>();
        extendInfo.put(OracleConstants.ORACLE_HOME,OracleConstants.ORACLE_HOME);
        extendInfo.put(OracleConstants.IS_ASM_INST,OracleConstants.IS_ASM_INST);
        extendInfo.put(OracleConstants.DB_ROLE,OracleConstants.DB_ROLE);
        extendInfo.put(OracleConstants.INST_NAME,OracleConstants.INST_NAME);
        extendInfo.put(DatabaseConstants.VERSION,DatabaseConstants.VERSION);
        extendInfo.put(OracleConstants.ORACLE_IP_INFOS,OracleConstants.ORACLE_IP_INFOS);
        extendInfo.put(OracleConstants.ORACLE_GROUP, OracleConstants.ORACLE_GROUP);
        return extendInfo;
    }

    private List<TaskEnvironment> mockEnvList() {
        List<TaskEnvironment> environments = new ArrayList<>();
        TaskEnvironment environment = new TaskEnvironment();
        environment.setUuid("envUuid");
        environments.add(environment);
        return environments;
    }

    private List<ProtectedResource> mockResourceList() {
        List<ProtectedResource> resourceList = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        resource.setAuth(new Authentication());
        resource.setExtendInfoByKey(OracleConstants.ORACLE_HOME, "test oracle.");
        resourceList.add(resource);
        return resourceList;
    }

    private Optional<ProtectedResource> getOptionalEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("envUuid");
        return Optional.of(environment);
    }
}