/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.sqlserver.protection.restore;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * sqlserver集群实例恢复测试类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-21
 */
@RunWith(SpringRunner.class)
@ComponentScan("com.huawei.oceanprotect.sqlserver.protection.restore")
@SpringBootTest(classes = {SqlServerGroupRestoreProvider.class, SqlServerBaseService.class})
public class SqlServerGroupRestoreProviderTest {
    @Autowired
    private SqlServerGroupRestoreProvider sqlServerGroupRestoreProvider;

    @Autowired
    private SqlServerBaseService sqlServerBaseService;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    protected ProviderManager providerManager;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    private AgentUnifiedRestApi agentUnifiedRestApi;


    @MockBean
    @Qualifier("unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @MockBean
    private RepositoryStrategyManager repositoryStrategyManager;

    @Before
    public void init() throws IllegalAccessException {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        copy.setBackupType(1);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.singletonList(getClusterInstance().get()));
        pageListResponse.setTotalCount(2);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
    }

    /**
     * 用例场景：实例恢复bean set注入
     * 前置条件：实例备份正常
     * 检 查 点：实例恢复bean set注入成功
     */
    @Test
    public void setAutowiredProperties_success() {
        sqlServerGroupRestoreProvider.setSqlServerBaseService(sqlServerBaseService);
        sqlServerGroupRestoreProvider.setResourceService(resourceService);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：实例恢复provider过滤
     * 前置条件：实例备份正常
     * 检 查 点：实例恢复provider过滤
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(
            sqlServerGroupRestoreProvider.applicable(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()));
    }

    /**
     * 用例场景：下发集群实例恢复任务
     * 前置条件：1. 资源是集群实例
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void cluster_instance_restore_intercept_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE,
            RestoreLocationEnum.ORIGINAL);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getClusterInstance());
        sqlServerGroupRestoreProvider.initialize(task);
        Assert.assertEquals(task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SINGLE.getType());
    }

    /**
     * 用例场景：下发可用性组原位置恢复任务
     * 前置条件：1. 资源是可用性组
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void always_on_original_restore_intercept_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON,
            RestoreLocationEnum.ORIGINAL);
        task.setRestoreType(RestoreTypeEnum.CR.getType());
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getClusterInstance());
        sqlServerGroupRestoreProvider.initialize(task);
        Assert.assertEquals(task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.AP.getType());
    }

    /**
     * 用例场景：下发可用性组细粒度新位置恢复成功（细粒度新位置目标对象是单实例）
     * 前置条件：1. 资源是单实例
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void always_on_fine_grained_restore_on_new_target_intercept_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_INSTANCE, RestoreLocationEnum.NEW);
        task.setRestoreType(RestoreTypeEnum.FLR.getType());
        task.setSubObjects(new ArrayList<>());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getClusterInstance());
        RestoreTask result = sqlServerGroupRestoreProvider.initialize(task);
        Assert.assertEquals(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(), result.getTargetObject().getSubType());
    }

    /**
     * 用例场景：下发可用性组新位置恢复任务
     * 前置条件：1. 资源是可用性组
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void always_on_new_restore_intercept_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE,
            RestoreLocationEnum.NEW);
        task.setTargetLocation(RestoreLocationEnum.NEW);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getClusterInstance());
        sqlServerGroupRestoreProvider.initialize(task);
        Assert.assertEquals(task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.AP.getType());
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1. 可用性组副本数据库级别恢复到新位置，targetObj是新单实例-->锁：新位置单实例资源
     * 2.  集群实例数据库级别恢复到新位置，targetObj是新单实例-->锁：新位置单实例资源
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void group_flr_new_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_INSTANCE, RestoreLocationEnum.NEW);
        task.setRestoreType(RestoreTypeEnum.FLR.getType());
        ProtectedResource resource = getClusterInstance().get();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        List<LockResourceBo> lockResources = sqlServerGroupRestoreProvider.getLockResources(task);
        Assert.assertEquals(1, lockResources.size());
    }

    /**
     * 用例场景：可用性组副本恢复到原位置，targetObj是原可用性组-->锁：原位置的可用性组资源、关联的实例加锁
     * 前置条件：1. 资源是集群实例或可用性组
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void group_normal_origin_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON,
            RestoreLocationEnum.ORIGINAL);
        task.setRestoreType(RestoreTypeEnum.CR.getType());
        ProtectedResource alwaysOnResource = getClusterInstance().get();
        alwaysOnResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
        PowerMockito.when(resourceService.getResourceById(anyString()))
            .thenReturn(Optional.ofNullable(alwaysOnResource));
        List<LockResourceBo> lockResources = sqlServerGroupRestoreProvider.getLockResources(task);
        Assert.assertEquals(2, lockResources.size());
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1. 可用性组副本数据库级别恢复到原位置，targetObj是原可用性组-->锁：原位置的可用性组资源、关联的实例加锁
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void group_flr_origin_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON,
            RestoreLocationEnum.ORIGINAL);
        task.setRestoreType(RestoreTypeEnum.FLR.getType());
        ProtectedResource alwaysOnResource = getClusterInstance().get();
        alwaysOnResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
        PowerMockito.when(resourceService.getResourceById(anyString()))
            .thenReturn(Optional.ofNullable(alwaysOnResource));
        List<LockResourceBo> lockResources = sqlServerGroupRestoreProvider.getLockResources(task);
        Assert.assertEquals(2, lockResources.size());
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1. 集群实例恢复到原位置，targetObj是原集群实例-->锁：原位置的集群实例资源、实例下所有数据库
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void cluster_instance_normal_origin_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE,
            RestoreLocationEnum.ORIGINAL);
        task.setRestoreType(RestoreTypeEnum.CR.getType());
        ProtectedResource resource = getClusterInstance().get();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        PowerMockito.when(resourceService.queryRelatedResourceUuids(anyString(), any()))
            .thenReturn(new HashSet<>(Collections.singleton(UUID.randomUUID().toString())));
        List<LockResourceBo> lockResources = sqlServerGroupRestoreProvider.getLockResources(task);
        Assert.assertEquals(2, lockResources.size());
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1. 集群实例数据库级别恢复到原位置，targetObj是原集群实例-->锁：原位置集群实例资源，实例下所有数据库
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void cluster_instance_flr_origin_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE,
            RestoreLocationEnum.ORIGINAL);
        task.setRestoreType(RestoreTypeEnum.FLR.getType());
        ProtectedResource resource = getClusterInstance().get();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        PowerMockito.when(resourceService.queryRelatedResourceUuids(anyString(), any()))
            .thenReturn(new HashSet<>(Collections.singleton(UUID.randomUUID().toString())));
        List<LockResourceBo> lockResources = sqlServerGroupRestoreProvider.getLockResources(task);
        Assert.assertEquals(2, lockResources.size());
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1. 可用性组副本恢复到新位置，targetObj是新集群实例-->锁：集群下所有的实例
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void group_normal_new_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE,
            RestoreLocationEnum.NEW);
        task.setRestoreType(RestoreTypeEnum.CR.getType());
        ProtectedResource resource = getClusterInstance().get();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        ProtectedResource instanceResource = new ProtectedResource();
        instanceResource.setUuid("123");
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>(Collections.singleton(instanceResource)));
        PowerMockito.when(resourceService.query(any())).thenReturn(response);
        List<LockResourceBo> lockResources = sqlServerGroupRestoreProvider.getLockResources(task);
        Assert.assertEquals(2, lockResources.size());
    }

    /**
     * 用例场景：下发可用性组原位置恢复任务,恢复到新位置，实例数量只有1个拋錯
     * 前置条件：1. 资源是可用性组
     * 检 查 点：恢复到的目标集群的实例是否有大于等于两个
     */
    @Test
    public void should_thrown_exception_if_always_on_original_restore_instance_is_insufficient() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON, RestoreLocationEnum.NEW);
        task.getTargetObject().setSubType(null);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getClusterInstance());
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        Assert.assertThrows(DataProtectionAccessException.class, () -> sqlServerGroupRestoreProvider.initialize(task));
    }

    /**
     * 用例场景：下发可用性组细粒度恢复，恢复到原位置恢复任务
     * 前置条件：1. 资源是可用性组
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void always_on_original_flr_restore_intercept_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON,
            RestoreLocationEnum.ORIGINAL);
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        task.setRestoreType(RestoreTypeEnum.FLR.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getClusterInstance());
        sqlServerGroupRestoreProvider.initialize(task);
        Assert.assertEquals(task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.AP.getType());
    }

    private Optional<ProtectedResource> getClusterInstance() {
        ProtectedResource clusterInstance = new ProtectedResource();
        ProtectedResource subInstance = new ProtectedResource();
        Authentication authentication = new Authentication();
        subInstance.setAuth(authentication);
        subInstance.setUuid(UUID.randomUUID().toString());
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        dependency.put(DatabaseConstants.AGENTS, Collections.singletonList(subInstance));
        dependency.put(DatabaseConstants.INSTANCE, Collections.singletonList(subInstance));
        clusterInstance.setDependencies(dependency);
        clusterInstance.setUuid(UUID.randomUUID().toString());
        return Optional.of(clusterInstance);
    }

    private RestoreTask getDatabaseRestoreTask(ResourceSubTypeEnum subTypeEnum,
        RestoreLocationEnum restoreLocationEnum) {
        RestoreTask task = new RestoreTask();
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        repositories.add(storageRepository);
        task.setRepositories(repositories);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setNodes(new ArrayList<>());
        Map<String, String> taskEnvironmentExtendInfo = new HashMap<>();
        taskEnvironment.setExtendInfo(taskEnvironmentExtendInfo);
        task.setTargetEnv(taskEnvironment);
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUID.randomUUID().toString());
        taskResource.setParentUuid(UUID.randomUUID().toString());
        taskResource.setSubType(subTypeEnum.getType());
        task.setTargetObject(taskResource);
        task.setAgents(new ArrayList<>());
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_PATH, "D:\\txt");
        task.setAdvanceParams(advanceParams);
        task.setTargetLocation(restoreLocationEnum);
        return task;
    }
}