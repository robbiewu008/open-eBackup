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
package openbackup.sqlserver.protection.restore;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * sqlserver数据库恢复测试类
 *
 */
@RunWith(SpringRunner.class)
@ComponentScan("com.huawei.oceanprotect.sqlserver.protection.restore")
@SpringBootTest(classes = {SqlServerDatabaseRestoreProvider.class, SqlServerBaseService.class})
public class SqlServerDatabaseRestoreProviderTest {
    @Autowired
    private SqlServerDatabaseRestoreProvider sqlServerDatabaseRestoreProvider;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    @Qualifier("unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    private AgentUnifiedRestApi agentUnifiedRestApi;

    @MockBean
    private RepositoryStrategyManager repositoryStrategyManager;


    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(getCopy());
    }

    /**
     * 用例场景：sqlserver数据库恢复类
     * 前置条件：数据库备份正常
     * 检 查 点：sqlserver数据库恢复类过滤成功
     */
    @Test
    public void applicable_success() {
        String subType = ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType();
        Assert.assertTrue(sqlServerDatabaseRestoreProvider.applicable(subType));
    }

    /**
     * 用例场景：单机部署原位置数据库恢复任务
     * 前置条件：1. 资源是单实例数据库
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void single_deploy_origin_database_restore_intercept_success() {
        RestoreTask task = getRestoreTask(CopyGeneratedByEnum.BY_LIVE_MOUNTE.value(), DatabaseDeployTypeEnum.SINGLE,
            RestoreLocationEnum.ORIGINAL);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getInstanceResource());
        sqlServerDatabaseRestoreProvider.supplyRestoreTask(task);
        Assert.assertEquals(task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SINGLE.getType());
    }

    /**
     * 用例场景：单机部署新位置数据库恢复任务
     * 前置条件：1. 资源是单实例数据库
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void single_deploy_new_database_restore_intercept_success() {
        RestoreTask task = getRestoreTask(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value(), DatabaseDeployTypeEnum.SINGLE,
            RestoreLocationEnum.NEW);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getInstanceResource());
        sqlServerDatabaseRestoreProvider.supplyRestoreTask(task);
        Assert.assertEquals(task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SINGLE.getType());
    }

    /**
     * 用例场景：恢复锁住资源成功,恢复到原位置，需要锁原位置数据库资源和原位置实例资源
     * 前置条件：1. 资源是数据库资源
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void database_origin_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_DATABASE, DatabaseDeployTypeEnum.SINGLE,
            RestoreLocationEnum.ORIGINAL);
        List<LockResourceBo> lockResources = sqlServerDatabaseRestoreProvider.getLockResources(task);
        System.out.println(JSONArray.fromObject(lockResources).toString());
        Assert.assertEquals(2, lockResources.size());
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1. 资源是数据库资源
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void database_new_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_DATABASE, DatabaseDeployTypeEnum.SINGLE,
            RestoreLocationEnum.NEW);
        List<LockResourceBo> lockResources = sqlServerDatabaseRestoreProvider.getLockResources(task);
        System.out.println(JSONArray.fromObject(lockResources).toString());
        Assert.assertEquals(1, lockResources.size());
    }

    /**
     * 用例场景：单机部署原位置数据库恢复任务
     * 前置条件：1. 资源是单实例数据库
     * 检 查 点：1. 恢复重命名数据库参数是否有问题
     */
    @Test
    public void throw_exception_single_deploy_new_database_name_replication() {
        RestoreTask task = getRestoreTask(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value(), DatabaseDeployTypeEnum.SINGLE,
            RestoreLocationEnum.ORIGINAL);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(getInstanceResource());
        task.getAdvanceParams().put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_NAME, "---ddd123&^%");
        Assert.assertThrows(LegoCheckedException.class, () -> sqlServerDatabaseRestoreProvider.supplyRestoreTask(task));
        task.getAdvanceParams().put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_NAME, "master");
        Assert.assertThrows(LegoCheckedException.class, () -> sqlServerDatabaseRestoreProvider.supplyRestoreTask(task));
        task.getAdvanceParams().put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_NAME, "resource1");
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(getProtectedResources());
        Assert.assertThrows(LegoCheckedException.class, () -> sqlServerDatabaseRestoreProvider.supplyRestoreTask(task));
        task.getAdvanceParams().put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_NAME, "resource");
        task.getTargetObject().setPath("res/123");
        task.getTargetObject().setName("res");
        sqlServerDatabaseRestoreProvider.supplyRestoreTask(task);
    }

    private PageListResponse<ProtectedResource> getProtectedResources() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setName("resource1");
        ProtectedResource resource2 = new ProtectedResource();
        resource2.setName("resource2");
        response.setRecords(Arrays.asList(resource1, resource2));
        response.setTotalCount(2);
        return response;
    }

    private RestoreTask getRestoreTask(String copyType, DatabaseDeployTypeEnum deployTypeEnum,
        RestoreLocationEnum restoreLocationEnum) {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_DATABASE, deployTypeEnum,
            restoreLocationEnum);
        task.setCopyId(UUID.randomUUID().toString());
        Copy copy = new Copy();
        copy.setGeneratedBy(copyType);
        List<TaskEnvironment> nodes = new ArrayList<>();
        TaskEnvironment node = new TaskEnvironment();
        node.setEndpoint("127.0.0.1");
        node.setPort(1);
        node.setUuid("111111");
        nodes.add(node);
        task.getTargetEnv().setNodes(nodes);
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_PATH,"D:\\txt");
        task.setAdvanceParams(advanceParams);
        return task;
    }

    private Optional<ProtectedResource> getInstanceResource () {
        ProtectedResource instance = new ProtectedResource();
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setEndpoint("127.0.0.1");
        env.setPort(3306);
        HashMap<String, String> extendInfo = new HashMap<>();
        env.setExtendInfo(extendInfo);
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        dependency.put(DatabaseConstants.AGENTS, Collections.singletonList(env));
        instance.setDependencies(dependency);
        instance.setPath("res/123");
        instance.setName("res");
        return Optional.of(instance);
    }

    private RestoreTask getDatabaseRestoreTask(ResourceSubTypeEnum subTypeEnum, DatabaseDeployTypeEnum deployTypeEnum,
        RestoreLocationEnum restoreLocationEnum) {
        RestoreTask task = new RestoreTask();
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        repositories.add(storageRepository);
        task.setRepositories(repositories);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        taskEnvironment.setNodes(new ArrayList<>());
        Map<String, String> taskEnvironmentExtendInfo = new HashMap<>();
        taskEnvironmentExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, deployTypeEnum.getType());
        taskEnvironment.setExtendInfo(taskEnvironmentExtendInfo);
        task.setTargetEnv(taskEnvironment);
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUID.randomUUID().toString());
        taskResource.setParentUuid(UUID.randomUUID().toString());
        taskResource.setSubType(subTypeEnum.getType());
        task.setTargetObject(taskResource);
        task.setAgents(new ArrayList<>());
        task.setTargetLocation(restoreLocationEnum);
        return task;
    }

    private Copy getCopy() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        copy.setBackupType(2);
        return copy;
    }
}
