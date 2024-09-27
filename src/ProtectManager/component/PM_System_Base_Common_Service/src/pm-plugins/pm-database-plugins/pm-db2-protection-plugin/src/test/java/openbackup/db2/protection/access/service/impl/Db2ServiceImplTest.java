/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.db2.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.DatabaseRestoreService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.enums.Db2ResourceStatusEnum;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link Db2ServiceImpl} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-13
 */
public class Db2ServiceImplTest {
    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final InstanceProtectionService instanceProtectionService = PowerMockito.mock(
        InstanceProtectionService.class);

    private final DatabaseRestoreService databaseRestoreService = PowerMockito.mock(DatabaseRestoreService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private Db2InstanceService db2instanceService = PowerMockito.mock(Db2InstanceService.class);

    private Db2ServiceImpl db2ServiceImpl = new Db2ServiceImpl(instanceResourceService, instanceProtectionService,
        databaseRestoreService, copyRestApi, resourceService);

    @Before
    public void init() {
        db2ServiceImpl.setAgentUnifiedService(agentUnifiedService);
        db2ServiceImpl.setDb2instanceService(db2instanceService);
    }

    /**
     * 用例场景：单机数据库获取agent信息
     * 前置条件：单机数据库
     * 检查点：获取正确的agent信息
     */
    @Test
    public void get_agents_when_single_database_by_instance_resource() {
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockInstanceResource(ResourceSubTypeEnum.DB2_INSTANCE.getType()));
        PowerMockito.when(instanceProtectionService.extractEnvNodesBySingleInstance(any()))
            .thenReturn(Arrays.asList(mockSingleEnv()));
        List<Endpoint> agents = db2ServiceImpl.getAgentsByInstanceResource(
            mockResource(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertEquals(1, agents.size());
    }

    /**
     * 用例场景：单机表空间获取agent信息
     * 前置条件：单机表空间
     * 检查点：获取正确的agent信息
     */
    @Test
    public void get_agents_when_single_tablespace_by_instance_resource() {
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockInstanceResource(ResourceSubTypeEnum.DB2_INSTANCE.getType()));
        PowerMockito.when(instanceProtectionService.extractEnvNodesBySingleInstance(any()))
            .thenReturn(Arrays.asList(mockSingleEnv()));
        List<Endpoint> agents = db2ServiceImpl.getAgentsByInstanceResource(
            mockResource(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        Assert.assertEquals(1, agents.size());
    }

    /**
     * 用例场景：集群数据库获取agent信息
     * 前置条件：集群数据库
     * 检查点：获取正确的agent信息
     */
    @Test
    public void get_agents_when_cluster_database_by_instance_resource() {
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockInstanceResource(ResourceSubTypeEnum.DB2_CLUSTER_INSTANCE.getType()));
        PowerMockito.when(instanceProtectionService.extractEnvNodesByClusterInstance(any()))
            .thenReturn(mockClusterEnv());
        List<Endpoint> agents = db2ServiceImpl.getAgentsByInstanceResource(
            mockResource(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertEquals(2, agents.size());
    }

    /**
     * 用例场景：集群表空间获取agent信息
     * 前置条件：集群表空间
     * 检查点：获取正确的agent信息
     */
    @Test
    public void get_agents_when_cluster_tablespace_by_instance_resource() {
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockInstanceResource(ResourceSubTypeEnum.DB2_CLUSTER_INSTANCE.getType()));
        PowerMockito.when(instanceProtectionService.extractEnvNodesByClusterInstance(any()))
            .thenReturn(mockClusterEnv());
        List<Endpoint> agents = db2ServiceImpl.getAgentsByInstanceResource(
            mockResource(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        Assert.assertEquals(2, agents.size());
    }

    /**
     * 用例场景：检查是否支持恢复
     * 前置条件：支持恢复
     * 检查点：检查通过
     */
    @Test
    public void checkSupportRestore() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockResource(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        db2ServiceImpl.checkSupportRestore(mockRestoreTask());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查表空间是否支持恢复
     * 前置条件：不支持时间点恢复
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_tablespace_timestamp_restore_when_check_tablespace_restore() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockResource(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        RestoreTask task = mockRestoreTask();
        task.getAdvanceParams().put(DatabaseConstants.RESTORE_TIME_STAMP_KEY, "111111");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2ServiceImpl.checkSupportRestore(task));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查表空间是否支持恢复
     * 前置条件：不支持新位置恢复
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_tablespace_new_restore_when_check_tablespace_restore() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockResource(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        RestoreTask task = mockRestoreTask();
        task.setTargetLocation(RestoreLocationEnum.NEW);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2ServiceImpl.checkSupportRestore(task));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查实例名是否一致
     * 前置条件：不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_instance_is_inconsistent_when_check_instance_name() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockResource(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        RestoreTask task = mockRestoreTask();
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2ServiceImpl.checkSupportRestore(task));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群节点数据是否一致
     * 前置条件：不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_is_inconsistent_when_check_node_num() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockResource(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        RestoreTask task = mockRestoreTask();
        task.getTargetObject().setParentName("db2inst1");
        task.getTargetEnv().setEndpoint("127.0.0.1,127.0.0.2");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2ServiceImpl.checkSupportRestore(task));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查恢复到新位置时数据库名称
     * 前置条件：选择了原数据库
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_database_is_consistent_when_check_database_name() {
        Copy copy = mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType());
        copy.setResourceId("921cf09d-f31b-4cd6-bf4e-04d6ccdfae8d");
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(copy);
        PowerMockito.when(instanceResourceService.getResourceById(any()))
            .thenReturn(mockResource(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        RestoreTask task = mockRestoreTask();
        task.getTargetObject().setParentName("db2inst1");
        task.getTargetObject().setUuid("921cf09d-f31b-4cd6-bf4e-04d6ccdfae8d");
        task.getTargetEnv().setEndpoint("127.0.0.1");
        task.setTargetLocation(RestoreLocationEnum.NEW);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2ServiceImpl.checkSupportRestore(task));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：更新hadr数据库状态
     * 前置条件：更新的是hadr
     * 检查点：无异常抛出
     */
    @Test
    public void execute_update_hadr_database_status_success() {
        db2ServiceImpl.updateHadrDatabaseStatus(mockTaskResource(), Db2ResourceStatusEnum.NORMAL.getStatus());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：查询数据库的数据量大小失败
     * 前置条件：查询接口失败
     * 检查点：返回为空
     */
    @Test
    public void should_return_empty_if_query_interface_fail_when_query_database_size() {
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(mockClusterInstance());
        PowerMockito.when(agentUnifiedService.getDetailPageList(any(), any(), any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error"));
        db2ServiceImpl.queryDatabaseSize(mockResource(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：查询数据库的数据量大小为空
     * 前置条件：查询接口返回结果为空
     * 检查点：返回为空
     */
    @Test
    public void should_return_empty_if_interface_return_empty_when_query_database_size() {
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(mockClusterInstance());
        PowerMockito.when(agentUnifiedService.getDetailPageList(any(), any(), any(), any())).thenReturn(mockDataSize());
        db2ServiceImpl.queryDatabaseSize(mockResource(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertTrue(true);
    }

    private TaskResource mockTaskResource() {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.HADR.getType());
        TaskResource protectObject = new TaskResource();
        protectObject.setExtendInfo(extendInfo);
        return protectObject;
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask task = new RestoreTask();
        task.setCopyId(UUID.randomUUID().toString());
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUID.randomUUID().toString());
        taskResource.setSubType(ResourceSubTypeEnum.DB2_DATABASE.getType());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, "dpf");
        extendInfo.put(DatabaseConstants.DEPLOY_OS_KEY, "suse");
        taskResource.setExtendInfo(extendInfo);
        task.setTargetObject(taskResource);
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        task.setAdvanceParams(new HashMap<>());
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setEndpoint("127.0.0.1");
        task.setTargetEnv(taskEnvironment);
        return task;
    }

    private Copy mockCopy(String subType) {
        Copy copy = new Copy();
        copy.setResourceSubType(subType);
        JSONObject resourceProperties = new JSONObject();
        resourceProperties.put(DatabaseConstants.VERSION, "10.5");
        JSONObject extendInfo = new JSONObject();
        extendInfo.put(DatabaseConstants.DEPLOY_OS_KEY, "suse");
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, "dpf");
        resourceProperties.put(DatabaseConstants.EXTEND_INFO, extendInfo);
        resourceProperties.put("parent_name", "db2inst1");
        copy.setResourceProperties(JSONObject.fromObject(resourceProperties).toString());
        copy.setResourceEnvironmentIp("127.0.0.1");
        return copy;
    }

    private ProtectedResource mockResource(String subType) {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(subType);
        resource.setParentUuid(UUID.randomUUID().toString());
        resource.setExtendInfoByKey(DatabaseConstants.INSTANCE_UUID_KEY, UUID.randomUUID().toString());
        resource.setVersion("10.5");
        resource.setParentName("db2inst2");
        return resource;
    }

    private ProtectedResource mockInstanceResource(String subType) {
        ProtectedResource instance = new ProtectedResource();
        instance.setSubType(subType);
        return instance;
    }

    private TaskEnvironment mockSingleEnv() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setEndpoint("127.0.0.1");
        taskEnvironment.setPort(50000);
        return taskEnvironment;
    }

    private List<TaskEnvironment> mockClusterEnv() {
        return Arrays.asList(mockSingleEnv(), mockSingleEnv());
    }

    private ProtectedResource mockDatabase() {
        ProtectedResource database = new ProtectedResource();
        database.setUuid(UUID.randomUUID().toString());
        database.setParentUuid(UUID.randomUUID().toString());
        return database;
    }

    private ProtectedResource mockClusterInstance() {
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setUuid(UUID.randomUUID().toString());
        clusterInstance.setParentUuid(UUID.randomUUID().toString());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("127.0.0.1");
        agent.setPort(50000);
        agent.setExtendInfoByKey(ResourceConstants.AGENT_IP_LIST, "127.0.0.1");
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agent);
        Map<String, List<ProtectedResource>> agentMap = new HashMap<>();
        agentMap.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource instance = new ProtectedResource();
        instance.setDependencies(agentMap);
        List<ProtectedResource> children = new ArrayList<>();
        children.add(instance);
        Map<String, List<ProtectedResource>> childrenMap = new HashMap<>();
        childrenMap.put(DatabaseConstants.CHILDREN, children);
        clusterInstance.setDependencies(childrenMap);
        clusterInstance.setExtendInfoByKey(Db2Constants.CATALOG_IP_KEY, "127.0.0.1");
        return clusterInstance;
    }

    private PageListResponse<ProtectedResource> mockDataSize() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(new ArrayList<>());
        return pageListResponse;
    }
}