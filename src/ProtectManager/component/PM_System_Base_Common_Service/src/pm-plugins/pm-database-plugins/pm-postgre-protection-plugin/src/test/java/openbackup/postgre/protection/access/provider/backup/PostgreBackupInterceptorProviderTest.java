/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.postgre.protection.access.provider.backup;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * postgre备份拦截器测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-29
 */
public class PostgreBackupInterceptorProviderTest {
    private final static String endpoint = "8.40.99.127";

    private final static Integer port = 52397;

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final PostgreBackupInterceptorProvider provider = new PostgreBackupInterceptorProvider(resourceService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_postgre_backup_interceptor_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：备份环境设置部署类型
     * 前置条件：保护对象为单实例
     * 检查点：是否返回单机部署
     */
    @Test
    public void should_return_single_deploy_if_single_instance() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        BackupTask returnTask = provider.supplyBackupTask(backupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            returnTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：备份环境设置部署类型
     * 前置条件：保护对象为主备集群实例
     * 检查点：是否返回主备部署
     */
    @Test
    public void should_return_AP_deploy_if_cluster_instance() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockClusterResource());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
        BackupTask returnTask = provider.supplyBackupTask(backupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(),
            returnTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：备份环境设置节点信息
     * 前置条件：无
     * 检查点：节点信息是否设置
     */
    @Test
    public void execute_supply_nodes_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockEnv());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        provider.supplyNodes(backupTask);
        Assert.assertEquals(IsmNumberConstant.ONE, backupTask.getProtectEnv().getNodes().size());
    }

    /**
     * 用例场景：备份环境设置集群节点信息
     * 前置条件：集群实例
     * 检查点：节点信息是否设置
     */
    @Test
    public void when_cluster_instance_execute_supply_nodes_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockClusterResource());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
        provider.supplyNodes(backupTask);
        Assert.assertEquals(IsmNumberConstant.TWO, backupTask.getProtectEnv().getNodes().size());
    }

    /**
     * 用例场景：备份环境设置存储仓库
     * 前置条件：日志备份
     * 检查点：是否设置data、log和cache仓库
     */
    @Test
    public void should_return_data_log_cache_repository_if_log_backup() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        BackupTask returnTask = provider.supplyBackupTask(backupTask);
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.LOG.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ZERO).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.CACHE.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ONE).getType().toString());
    }

    /**
     * 用例场景：备份环境设置存储仓库
     * 前置条件：全量备份
     * 检查点：是否设置data和cache仓库
     */
    @Test
    public void should_return_data_cache_repository_if_full_backup() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        backupTask.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        BackupTask returnTask = provider.supplyBackupTask(backupTask);
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.DATA.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ZERO).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.CACHE.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ONE).getType().toString());
    }

    /**
     * 用例场景：备份环境设置agents信息
     * 前置条件：无
     * 检查点：agents信息是否设置
     */
    @Test
    public void execute_supply_agent_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockEnv());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        provider.supplyAgent(backupTask);
        Assert.assertEquals(IsmNumberConstant.ONE, backupTask.getAgents().size());
    }

    private BackupTask mockBackupTask(String subType) {
        BackupTask backupTask = new BackupTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(subType);
        backupTask.setProtectObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setExtendInfo(new HashMap());
        backupTask.setProtectEnv(taskEnvironment);
        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        backupTask.addRepository(dataRepository);
        return backupTask;
    }

    private Optional<ProtectedResource> mockEnv() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint(endpoint);
        protectedEnvironment.setPort(port);
        protectedEnvironment.setExtendInfo(new HashMap<>());
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(protectedEnvironment);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(dependencies);
        environment.setExtendInfo(new HashMap<>());
        return Optional.of(environment);
    }

    private Optional<ProtectedResource> mockClusterResource() {
        ProtectedResource protectedResourceOne = mockEnv().get();
        ProtectedResource protectedResourceTwo = mockEnv().get();
        List<ProtectedResource> children = new ArrayList<>();
        children.add(protectedResourceOne);
        children.add(protectedResourceTwo);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, children);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(dependencies);
        ProtectedResource resource = new ProtectedResource();
        resource.setDependencies(dependencies);
        return Optional.of(environment);
    }
}