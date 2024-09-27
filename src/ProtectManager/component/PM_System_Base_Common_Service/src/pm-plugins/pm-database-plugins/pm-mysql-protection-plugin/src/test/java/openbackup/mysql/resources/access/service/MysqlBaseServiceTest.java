/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.mysql.resources.access.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.interceptor.MysqlBaseMock;
import openbackup.mysql.resources.access.provider.config.MysqlAgentProxyConfig;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * mysql应用基本的Service 测试类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/6/16
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class})
public class MysqlBaseServiceTest {
    private static final String DMR_PROXY_IP = "8.40.99.187";

    private static final Integer DME_PROXY_PORT = 8090;

    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final EncryptorService encryptorService = Mockito.mock(EncryptorService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private MysqlBaseService mysqlBaseService = new MysqlBaseService(resourceService, agentUnifiedService, encryptorService
        , instanceResourceService);;

    @Before
    public void init() {
        MysqlAgentProxyConfig agentProxyProperties = new MysqlAgentProxyConfig();
        agentProxyProperties.setHost("10.44.218.91");
        agentProxyProperties.setPort(3306);
        PowerMockito.mockStatic(FeignBuilder.class);
        AgentUnifiedRestApi builder = Mockito.mock(AgentUnifiedRestApi.class);
        PowerMockito.when(
            FeignBuilder.buildHttpsTarget(eq(AgentUnifiedRestApi.class), any(), anyBoolean(), anyBoolean(), any()))
            .thenReturn(builder);
    }

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
        ProtectedEnvironment agentReturnEnv = mysqlBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid);
        Assert.assertTrue(agentReturnEnv.equals(agentEnv));
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. 没有Agent主机信息
     * 检 查 点：1. 报错，且错误信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_agent_when_agent_is_null() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("single instance dependency agent is not one.");
        String singleInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        dependencies.put("agents", null);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
                .thenReturn(Optional.of(protectedResource));
        mysqlBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid);
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. 存在多个Agent主机信息
     * 检 查 点：1. 报错，且错误信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_agent_when_agent_is_not_one() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("single instance dependency agent is not one.");
        String singleInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        List<ProtectedResource> agentResources = new ArrayList<>();
        agentResources.add(agentEnv);
        agentResources.add(agentEnv);
        dependencies.put("agents", agentResources);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
                .thenReturn(Optional.of(protectedResource));
        mysqlBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid);
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. Agent主机信息不是环境
     * 检 查 点：1. 报错，且错误信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_agent_when_agent_is_not_env() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("mysql agent resource is not env.");
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
        mysqlBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid);
    }

    /**
     * 用例场景：针对集群实例uuid，从dependency里，获取集群实例下面的所有子实例
     * 前置条件：1. 存在子实例信息
     * 检 查 点：1. 不报错且获取到子实例信息和预期一样
     */
    @Test
    public void get_single_instance_by_cluster_instance_success() {
        String clusterInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        List<ProtectedResource> singleInstances = new ArrayList<>();
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        singleInstances.add(agentEnv);
        dependencies.put("children", singleInstances);
        PowerMockito.when(resourceService.getResourceById(clusterInstanceUuid))
                .thenReturn(Optional.of(protectedResource));
        List<ProtectedResource> returnSingleInstances = mysqlBaseService.getSingleInstanceByClusterInstance(
                clusterInstanceUuid);
        Assert.assertTrue(singleInstances.size() == returnSingleInstances.size());
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
        Endpoint agentEndpoint = mysqlBaseService.getAgentEndpoint(env);
        Assert.assertTrue(agentEndpoint.getId().equals(env.getUuid()));
        Assert.assertTrue(agentEndpoint.getIp().equals(env.getEndpoint()));
        Assert.assertTrue(agentEndpoint.getPort() == env.getPort());
    }

    /**
     * 用例场景：针对Mysql集群实例里的所有单实例信息，将单实例信息的auth设置到单实例对应的nodes的auth中
     * 前置条件：1. AUTH信息存在
     * 检 查 点：1. 不报错且获取到auth信息和预期一样
     */
    @Test
    public void set_nodes_auth_success() {
        TaskEnvironment nodeEnvOne = new TaskEnvironment();
        nodeEnvOne.setUuid(UUID.randomUUID().toString());

        TaskEnvironment nodeEnvTwo = new TaskEnvironment();
        nodeEnvTwo.setUuid(UUID.randomUUID().toString());

        List<TaskEnvironment> nodes = Arrays.asList(nodeEnvOne, nodeEnvTwo);

        ProtectedResource singleInstanceResOne = new ProtectedResource();
        singleInstanceResOne.setDependencies(new HashMap<>());
        singleInstanceResOne.setUuid(nodeEnvOne.getUuid());
        singleInstanceResOne.setExtendInfo(new HashMap<>());
        singleInstanceResOne.getExtendInfo().put(DatabaseConstants.ROLE, "1");
        singleInstanceResOne.getExtendInfo().put(DatabaseConstants.DATA_DIR, "/opt/1");
        singleInstanceResOne.getExtendInfo().put(MysqlConstants.LOG_BIN_INDEX_PATH, "/log/1");
        singleInstanceResOne.getExtendInfo().put(MysqlConstants.INSTANCE_IP, "8.40.99.101");

        ProtectedResource singleInstanceResTwo = new ProtectedResource();
        singleInstanceResTwo.setDependencies(new HashMap<>());
        singleInstanceResTwo.setUuid(nodeEnvTwo.getUuid());
        singleInstanceResTwo.setExtendInfo(new HashMap<>());
        singleInstanceResTwo.getExtendInfo().put(DatabaseConstants.ROLE, "2");
        singleInstanceResTwo.getExtendInfo().put(DatabaseConstants.DATA_DIR, "/opt/2");
        singleInstanceResTwo.getExtendInfo().put(MysqlConstants.LOG_BIN_INDEX_PATH, "/log/1");
        singleInstanceResTwo.getExtendInfo().put(MysqlConstants.INSTANCE_IP, "8.40.99.102");


        ProtectedEnvironment envOne = new ProtectedEnvironment();
        envOne.setUuid(singleInstanceResOne.getUuid());
        Authentication authOne = new Authentication();
        authOne.setAuthKey("111");
        singleInstanceResOne.setAuth(authOne);

        ProtectedEnvironment envTwo = new ProtectedEnvironment();
        envTwo.setUuid(singleInstanceResTwo.getUuid());
        Authentication authTwo = new Authentication();
        authTwo.setAuthKey("222");
        singleInstanceResTwo.setAuth(authTwo);

        Map<String, List<ProtectedResource>> dependencyOne = singleInstanceResOne.getDependencies();
        dependencyOne.put(DatabaseConstants.AGENTS, Arrays.asList(envOne));

        Map<String, List<ProtectedResource>> dependencyTwo = singleInstanceResTwo.getDependencies();
        dependencyTwo.put(DatabaseConstants.AGENTS, Arrays.asList(envTwo));

        PowerMockito.when(resourceService.getResourceById(singleInstanceResOne.getUuid()))
                .thenReturn(Optional.of(singleInstanceResOne));
        PowerMockito.when(resourceService.getResourceById(singleInstanceResTwo.getUuid()))
                .thenReturn(Optional.of(singleInstanceResTwo));

        List<ProtectedResource> singleInstanceResources = Arrays.asList(singleInstanceResOne, singleInstanceResTwo);

        mysqlBaseService.setNodesAuth(nodes, singleInstanceResources);
        Assert.assertTrue(authOne.getAuthKey().equals(nodes.get(0).getAuth().getAuthKey()));
        Assert.assertTrue(authTwo.getAuthKey().equals(nodes.get(1).getAuth().getAuthKey()));
        Assert.assertTrue(singleInstanceResOne.getExtendInfo()
                .get(DatabaseConstants.ROLE)
                .equals(nodes.get(0).getExtendInfo().get(DatabaseConstants.ROLE)));
        Assert.assertTrue(singleInstanceResTwo.getExtendInfo()
                .get(DatabaseConstants.ROLE)
                .equals(nodes.get(1).getExtendInfo().get(DatabaseConstants.ROLE)));
        Assert.assertFalse(VerifyUtil.isEmpty((singleInstanceResOne.getExtendInfo()
                .get(DatabaseConstants.DATA_DIR))));
        Assert.assertFalse(singleInstanceResOne.getExtendInfo()
                .get(DatabaseConstants.DATA_DIR)
                .equals(nodes.get(0).getExtendInfo().get(DatabaseConstants.DATA_DIR)));
        Assert.assertFalse(singleInstanceResTwo.getExtendInfo()
                .get(DatabaseConstants.DATA_DIR)
                .equals(nodes.get(1).getExtendInfo().get(DatabaseConstants.DATA_DIR)));
        Assert.assertFalse(VerifyUtil.isEmpty((singleInstanceResOne.getExtendInfo()
                .get(MysqlConstants.LOG_BIN_INDEX_PATH))));
        Assert.assertFalse(singleInstanceResOne.getExtendInfo()
                .get(MysqlConstants.LOG_BIN_INDEX_PATH)
                .equals(nodes.get(0).getExtendInfo().get(MysqlConstants.LOG_BIN_INDEX_PATH)));
        Assert.assertFalse(singleInstanceResTwo.getExtendInfo()
                .get(MysqlConstants.LOG_BIN_INDEX_PATH)
                .equals(nodes.get(1).getExtendInfo().get(MysqlConstants.LOG_BIN_INDEX_PATH)));

        Assert.assertTrue(singleInstanceResOne.getExtendInfo()
                .get(MysqlConstants.INSTANCE_IP)
                .equals(nodes.get(0).getExtendInfo().get(MysqlConstants.INSTANCE_IP)));
        Assert.assertTrue(singleInstanceResTwo.getExtendInfo()
                .get(MysqlConstants.INSTANCE_IP)
                .equals(nodes.get(1).getExtendInfo().get(MysqlConstants.INSTANCE_IP)));
    }

    /**
     * 用例场景：检验集群role
     * 前置条件：1. 有一个主节点，一个被节点
     * 检 查 点：1. 不报错
     */
    @Test
    public void check_cluster_role_success() {
        TaskEnvironment masterNode = new TaskEnvironment();
        masterNode.setExtendInfo(new HashMap<>());
        masterNode.getExtendInfo().put(DatabaseConstants.ROLE, "1");

        TaskEnvironment slaveNode = new TaskEnvironment();
        slaveNode.setExtendInfo(new HashMap<>());
        slaveNode.getExtendInfo().put(DatabaseConstants.ROLE, "2");

        List<TaskEnvironment> nodes1 = Arrays.asList(masterNode, slaveNode);
        mysqlBaseService.checkClusterRole(nodes1);

        List<TaskEnvironment> nodes2 = Arrays.asList(masterNode);
        mysqlBaseService.checkClusterRole(nodes2);
        Assert.assertThrows(LegoCheckedException.class, ()-> mysqlBaseService.checkClusterRole(null));
    }

    /**
     * 用例场景：检验集群role
     * 前置条件：1. 有两个主节点
     * 检 查 点：1. 报错，且错误信息与预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_master_node_is_not_one_when_check_cluster_role() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("master node is not 1. is 2");
        TaskEnvironment masterNode = new TaskEnvironment();
        masterNode.setExtendInfo(new HashMap<>());
        masterNode.getExtendInfo().put(DatabaseConstants.ROLE, "1");

        TaskEnvironment slaveNode = new TaskEnvironment();
        slaveNode.setExtendInfo(new HashMap<>());
        slaveNode.getExtendInfo().put(DatabaseConstants.ROLE, "1");

        List<TaskEnvironment> nodes1 = Arrays.asList(masterNode, slaveNode);
        mysqlBaseService.checkClusterRole(nodes1);
    }

    /**
     * 用例场景：检验集群role
     * 前置条件：1. 没有节点
     * 检 查 点：1. 报错，且错误信息与预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_is_null_when_check_cluster_role() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("nodes is null.");
        mysqlBaseService.checkClusterRole(null);
    }

    /**
     * 用例场景：设置备份对象的version成功
     * 前置条件：1. 备份对象的extendInfo是null
     * 检 查 点：1. extendInfo不为null，且有值
     */
    @Test
    public void set_version_success_when_protect_object_extend_is_null() {
        String version = "111";
        final Map<String, String> extendInfoRes = mysqlBaseService.supplyExtendInfo(version, null);
        Assert.assertEquals(version, extendInfoRes.get(DatabaseConstants.VERSION));
    }

    /**
     * 用例场景：设置备份对象的version成功
     * 前置条件：1. 备份对象的extendInfo不是null
     * 检 查 点：1. extendInfo不为null，且有值
     */
    @Test
    public void set_version_success_when_protect_object_extend_is_not_null() {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("a", "b");
        String version = "111";
        final Map<String, String> extendInfoRes = mysqlBaseService.supplyExtendInfo(version, extendInfo);
        Assert.assertEquals(version, extendInfoRes.get(DatabaseConstants.VERSION));
        Assert.assertEquals("b", extendInfoRes.get("a"));
    }

    /**
     * 用例场景：查询数据库版本
     * 前置条件：1. 数据库服务正常，2. 查询出的数据库列表不为空
     * 检 查 点：1. 查询version成功
     */
    @Test
    public void get_version_success() throws IllegalAccessException {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3390);

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("1111111111111111");

        AppEnvResponse result = new AppEnvResponse();
        HashMap<String, String> ext = new HashMap<>();
        ext.put(DatabaseConstants.VERSION, "7.2.3");
        result.setExtendInfo(ext);

        String version = mysqlBaseService.queryDatabaseVersion(resource, protectedEnvironment);
        Assert.assertEquals(version, "7.2.3");
    }

    /**
     * 用例场景：查询数据库版本
     * 前置条件：1. 数据库服务异常，2. 查询出的数据库列表为空
     * 检 查 点：1. 查询version失败抛出异常
     */
    @Test
    public void get_version_when_mysql_service_broke() throws IllegalAccessException {
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3390);

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("1111111111111111");
        resource.setExtendInfo(new HashMap<>());

        AppEnvResponse result = new AppEnvResponse();
        mysqlBaseService.queryDatabaseVersion(resource, protectedEnvironment);
    }

    /**
     * 用例场景：查询MySQL的部署的操作系统
     * 前置条件：1. 数据库服务正常，2. 能查询到linux的部署操作系统
     * 检 查 点：1. 查询部署操作系统成功
     */
    @Test
    public void query_deploy_operating_system_success() throws IllegalAccessException {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3390);

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("1111111111111111");

        AppEnvResponse result = new AppEnvResponse();
        HashMap<String, String> ext = new HashMap<>();
        ext.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "centos");
        result.setExtendInfo(ext);

        String deployOperatingSystem = mysqlBaseService.queryDeployOperatingSystem(resource, protectedEnvironment);
        Assert.assertEquals(deployOperatingSystem, "centos");
    }

    /**
     * 用例场景：查询MySQL的部署的操作系统
     * 前置条件：1. 数据库服务异常，2. 查询失败
     * 检 查 点：1. 查询部署操作系统失败抛出异常
     */
    @Test
    public void query_deploy_operating_system_when_system_broke() throws IllegalAccessException {
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3390);

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("1111111111111111");
        resource.setExtendInfo(new HashMap<>());

        AppEnvResponse result = new AppEnvResponse();

        mysqlBaseService.queryDatabaseVersion(resource, protectedEnvironment);
    }

    /**
     * 用例场景：check部署系统
     * 前置条件：恢复任务参数下发
     * 检 查 点：1. check部署的操作系统相同
     */
    @Test
    public void check_deploy_operating_system_success() {
        // 操作系统拦截
        HashMap<String, String> targetResourceExtendInfo = new HashMap<>();
        targetResourceExtendInfo.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "centos");
        HashMap<String, String> copyExtendInfo = new HashMap<>();
        copyExtendInfo.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "centos");
        mysqlBaseService.checkDeployOperatingSystem(targetResourceExtendInfo, copyExtendInfo);
        copyExtendInfo.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "centos1");
        Assert.assertThrows(LegoCheckedException.class,
            ()-> mysqlBaseService.checkDeployOperatingSystem(targetResourceExtendInfo, copyExtendInfo));
    }

    /**
     * 用例场景：check部署系统
     * 前置条件：恢复任务参数下发，目标端与源端参数不一致
     * 检 查 点：1. check部署的操作系统不一致
     */
    @Test
    public void check_deploy_operating_system_when_get_system_null() {
        expectedException.expect(LegoCheckedException.class);
        // 操作系统拦截
        HashMap<String, String> targetResourceExtendInfo = new HashMap<>();
        targetResourceExtendInfo.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "centos");
        HashMap<String, String> copyExtendInfo = new HashMap<>();
        copyExtendInfo.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "SUSE");
        mysqlBaseService.checkDeployOperatingSystem(targetResourceExtendInfo, copyExtendInfo);
    }

    /**
     * 用例场景：check sub成功
     * 前置条件：恢复任务参数下发
     * 检 查 点：1. check部署的操作系统相同
     */
    @Test
    public void check_sub_type_success() {
        mysqlBaseService.checkSubType(MysqlBaseMock.getCopy(),
                MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE).getTargetObject());
        TaskResource targetResource = new TaskResource();
        targetResource.setSubType("1");
        Assert.assertThrows(LegoCheckedException.class, ()-> mysqlBaseService.checkSubType(MysqlBaseMock.getCopy(), targetResource));
    }

    /**
     * 用例场景：check sub失败且抛出异常
     * 前置条件：恢复任务参数下发
     * 检 查 点：1. check部署的操作系统不同
     */
    @Test
    public void check_sub_type_fail_should_throw_exception() {
        expectedException.expect(LegoCheckedException.class);
        mysqlBaseService.checkSubType(MysqlBaseMock.getCopy(),
                MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_DATABASE).getTargetObject());
    }

    /**
     * 用例场景：检验集群类型成功
     * 前置条件：备份成功，恢复任务参数下发
     * 检 查 点：1. 集群类型相同
     */
    @Test
    public void check_cluster_type_success() {
        HashMap<String, String> targetResourceExtendInfo = new HashMap<>();
        targetResourceExtendInfo.put(DatabaseConstants.CLUSTER_TYPE, "PXC");
        HashMap<String, String> copyExtendInfo = new HashMap<>();
        copyExtendInfo.put(DatabaseConstants.CLUSTER_TYPE, "PXC");
        mysqlBaseService.checkClusterType(targetResourceExtendInfo, copyExtendInfo);
        copyExtendInfo.put(DatabaseConstants.CLUSTER_TYPE, "PC");
        Assert.assertThrows(LegoCheckedException.class,
            ()-> mysqlBaseService.checkClusterType(targetResourceExtendInfo, copyExtendInfo));
    }

    /**
     * 用例场景：检验集群类型失败
     * 前置条件：备份成功，恢复任务参数下发
     * 检 查 点：1. 集群类型不一致，抛出异常
     */
    @Test
    public void check_cluster_type_fail_should_throw_exception() {
        expectedException.expect(LegoCheckedException.class);
        HashMap<String, String> targetResourceExtendInfo = new HashMap<>();
        targetResourceExtendInfo.put(DatabaseConstants.CLUSTER_TYPE, "PXC");
        HashMap<String, String> copyExtendInfo = new HashMap<>();
        copyExtendInfo.put(DatabaseConstants.CLUSTER_TYPE, "AP");
        mysqlBaseService.checkClusterType(targetResourceExtendInfo, copyExtendInfo);
    }

    /**
     * 用例场景：检验version失败
     * 前置条件：备份成功，恢复任务参数下发
     * 检 查 点：1. 源副本和目标资源版本不一致
     */
    @Test
    public void check_version_fail_should_throw_exception() {
        expectedException.expect(LegoCheckedException.class);
        mysqlBaseService.checkVersion(
                MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_DATABASE).getTargetObject(),
                JSONObject.fromObject(MysqlBaseMock.getCopy().getResourceProperties()));
    }

    /**
     * 用例场景：检验数据库恢复新名称成功
     * 前置条件：1、数据库备份成功
     * 检 查 点：1. 名称大于64个字符
     */
    @Test
    public void check_new_databases_name_success() {
        HashMap<String, String> advanceParams = new HashMap<>();
        advanceParams.put(MysqlConstants.NEW_DATABASE_NAME, "ceshi123");
        RestoreTask task = new RestoreTask();
        task.setAdvanceParams(advanceParams);
        mysqlBaseService.checkNewDatabaseName(task);
        advanceParams.put(MysqlConstants.NEW_DATABASE_NAME, "-|ceshi123\n");
        task.setAdvanceParams(advanceParams);
        Assert.assertThrows(LegoCheckedException.class, ()->mysqlBaseService.checkNewDatabaseName(task));
    }

    /**
     * 用例场景：检验数据库恢复新名称失败
     * 前置条件：1、数据库备份成功
     * 检 查 点：1. 名称大于64个字符
     */
    @Test
    public void check_new_databases_name_fail_when_length_is_invalid() {
        expectedException.expect(LegoCheckedException.class);
        HashMap<String, String> advanceParams = new HashMap<>();
        advanceParams.put(MysqlConstants.NEW_DATABASE_NAME,
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab");
        RestoreTask task = new RestoreTask();
        task.setAdvanceParams(advanceParams);
        mysqlBaseService.checkNewDatabaseName(task);
    }
    /**
     * 用例场景：检验数据库恢复原位置
     * 前置条件：1、数据库备份成功
     * 检 查 点：1. 原位置恢复
     */
    @Test
    public void check_new_databases_name_fail_when_name_is_null() {
        HashMap<String, String> advanceParams = new HashMap<>();
        RestoreTask task = new RestoreTask();
        task.setAdvanceParams(advanceParams);
        mysqlBaseService.checkNewDatabaseName(task);
        advanceParams.put(MysqlConstants.NEW_DATABASE_NAME, "-|ceshi123\n");
        task.setAdvanceParams(advanceParams);
        Assert.assertThrows(LegoCheckedException.class, ()->mysqlBaseService.checkNewDatabaseName(task));
    }

    /**
     * 用例场景：检验数据库恢复新名称失败
     * 前置条件：1、数据库备份成功
     * 检 查 点：1. 名称含有特殊名称
     */
    @Test
    public void check_new_databases_name_fail_when_path_is_invalid() {
        expectedException.expect(LegoCheckedException.class);
        HashMap<String, String> advanceParams = new HashMap<>();
        advanceParams.put(MysqlConstants.NEW_DATABASE_NAME, "abs}]#");
        RestoreTask task = new RestoreTask();
        task.setAdvanceParams(advanceParams);
        mysqlBaseService.checkNewDatabaseName(task);
    }

    /**
     * 用例场景：检验数据库恢复新名称失败
     * 前置条件：1、数据库备份成功
     * 检 查 点：1. 恢复到系统数据库
     */
    @Test
    public void check_new_databases_name_fail_when_database_is_exist() {
        expectedException.expect(LegoCheckedException.class);
        HashMap<String, String> advanceParams = new HashMap<>();
        advanceParams.put(MysqlConstants.NEW_DATABASE_NAME, "sys");
        RestoreTask task = new RestoreTask();
        task.setAdvanceParams(advanceParams);
        mysqlBaseService.checkNewDatabaseName(task);
    }

    /**
     * 用例场景：检验集群实例节点状态修改成功
     * 前置条件：1、集群实例注册成功
     * 检 查 点：1. 集群实例节点状态更改
     */
    @Test
    public void check_health_Check_All_Nodes_Of_ClusterInstance_InEnvironment_success(){
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstance());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockResource()));
        mysqlBaseService.healthCheckAllNodes(mockEnvironment());
        Mockito.verify(resourceService, Mockito.times(1)).getResourceById(any());
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setParentUuid(UUID.randomUUID().toString());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "5432");
        resource.setExtendInfo(extendInfo);
        ProtectedResource subInstance = new ProtectedResource();
        subInstance.setExtendInfoByKey(DatabaseConstants.HOST_ID, UUID.randomUUID().toString());
        subInstance.setExtendInfoByKey(DatabaseConstants.SERVICE_IP, "127.0.0.1");
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(DMR_PROXY_IP);
        environment.setPort(DME_PROXY_PORT);
        subInstance.setEnvironment(environment);
        subInstance.setAuth(new Authentication());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("d9c6a90e-e86d-473b-9d1d-793982b1c6c1");
        agent.setEndpoint(DMR_PROXY_IP);
        agent.setPort(DME_PROXY_PORT);
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agent);
        Map<String, List<ProtectedResource>> subDependencies = new HashMap();
        subDependencies.put(DatabaseConstants.AGENTS, agents);
        subInstance.setDependencies(subDependencies);
        List<ProtectedResource> children = new ArrayList<>();
        children.add(subInstance);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, children);
        resource.setDependencies(dependencies);
        resource.setEnvironment(mockEnv());
        return resource;
    }

    private ProtectedEnvironment mockEnv() {
        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid("d9c6a90e-e86d-473b-9d1d-793982b1c6c1");
        agentEnv.setEndpoint(DMR_PROXY_IP);
        agentEnv.setPort(DME_PROXY_PORT);
        agents.add(agentEnv);
        Map<String, List<ProtectedResource>> envDependencies = new HashMap<>();
        envDependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(envDependencies);
        return environment;
    }

    private PageListResponse<ProtectedResource> mockInstance() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setAuth(new Authentication());
        List<ProtectedResource> dataList = new ArrayList<>();
        dataList.add(protectedResource);
        PageListResponse<ProtectedResource> instances = new PageListResponse<>();
        instances.setRecords(dataList);
        return instances;
    }

    private ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setParentUuid(UUID.randomUUID().toString());
        return environment;
    }
}
