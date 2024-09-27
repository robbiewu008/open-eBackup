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
package openbackup.db2.protection.access.provider.restore;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

/**
 * {@link Db2RestoreInterceptorProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-14
 */
public class Db2RestoreInterceptorProviderTest {
    private static final String DB2VERSION = "DB2 V10.5.0.5";

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final Db2Service db2Service = PowerMockito.mock(Db2Service.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private Db2RestoreInterceptorProvider provider = new Db2RestoreInterceptorProvider(copyRestApi,
        instanceResourceService, db2Service, resourceService);

    /**
     * 用例场景：框架调applicable接口
     * 前置条件：输入db2资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_restore_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
    }

    /**
     * 用例场景：db2数据库恢复获取锁资源
     * 前置条件：资源正常
     * 检查点：是否拿到需要加锁的资源
     */
    @Test
    public void get_database_restore_lock_resources_success() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        PowerMockito.when(resourceService.queryRelatedResourceUuids(anyString(), any()))
            .thenReturn(mockLockTablespace());
        List<LockResourceBo> lockResourcesList = provider.getLockResources(mockRestoreTask());
        Assert.assertEquals(IsmNumberConstant.TWO, lockResourcesList.size());
    }

    /**
     * 用例场景：db2表空间恢复获取锁资源
     * 前置条件：资源正常
     * 检查点：是否拿到需要加锁的资源
     */
    @Test
    public void get_tablespace_restore_lock_resources_success() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        PowerMockito.when(resourceService.queryRelatedResourceUuids(anyString(), any()))
            .thenReturn(mockLockTablespace());
        List<LockResourceBo> lockResourcesList = provider.getLockResources(mockRestoreTask());
        Assert.assertEquals(IsmNumberConstant.TWO, lockResourcesList.size());
    }

    /**
     * 用例场景：db2恢复拦截方法设置参数
     * 前置条件：调用到该拦截方法
     * 检查点：参数是否设置正确
     */
    @Test
    public void execute_intercept_success() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(mockDatabaseResource());
        RestoreTask restoreTask = provider.initialize(mockRestoreTask());
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), restoreTask.getRestoreMode());
        Assert.assertEquals(DB2VERSION, restoreTask.getTargetObject().getExtendInfo().get(DatabaseConstants.VERSION));
        Assert.assertEquals(DB2VERSION,
            restoreTask.getAdvanceParams().get(DatabaseConstants.COPY_PROTECT_OBJECT_VERSION_KEY));
        Assert.assertEquals(RestoreLocationEnum.NEW.getLocation(),
            restoreTask.getAdvanceParams().get(DatabaseConstants.TARGET_LOCATION_KEY));
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(),
            restoreTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：db2恢复拦截方法设置目标对象
     * 前置条件：恢复到原位置
     * 检查点：参数是否设置正确
     */
    @Test
    public void should_return_latest_target_object_if_original_location_restore_when_intercept() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(mockDatabaseResource());
        RestoreTask task = mockRestoreTask();
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.HADR.getType());
        RestoreTask restoreTask = provider.initialize(task);
        Assert.assertEquals("127.0.0.1", restoreTask.getTargetObject().getExtendInfo().get("catalogIp"));
    }

    /**
     * 用例场景：hadr恢复拦截方法设置节点数据库
     * 前置条件：hadr数据库恢复
     * 检查点：参数是否设置正确
     */
    @Test
    public void should_return_node_database_if_hadr_restore_when_intercept() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(mockDatabaseResource());
        RestoreTask task = mockRestoreTask();
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.HADR.getType());
        RestoreTask restoreTask = provider.initialize(task);
        Assert.assertEquals("testdb", restoreTask.getTargetEnv().getExtendInfo().get(Db2Constants.NODE_DATABASE_KEY));
    }

    /**
     * 用例场景：db2数据库恢复获取转全量的关联资源
     * 前置条件：资源正常
     * 检查点：是否获取到转全量关联资源
     */
    @Test
    public void find_associated_resources_to_set_next_full_success() {
        PowerMockito.when(copyRestApi.queryCopyByID(any()))
            .thenReturn(mockCopy(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        PowerMockito.when(resourceService.queryRelatedResourceUuids(anyString(), any()))
            .thenReturn(mockLockTablespace());
        List<LockResourceBo> lockResourcesList = provider.getLockResources(mockRestoreTask());
        Assert.assertEquals(IsmNumberConstant.TWO, lockResourcesList.size());
    }

    /**
     * 用例场景：恢复后置处理
     * 前置条件：hadr数据库恢复
     * 检查点：无异常抛出
     */
    @Test
    public void execute_restore_post_process_success() {
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(mockDatabaseResource());
        provider.postProcess(mockRestoreTask(), ProviderJobStatusEnum.FAIL);
        Assert.assertTrue(true);
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask task = new RestoreTask();
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid(UUID.randomUUID().toString());
        targetObject.setSubType(ResourceSubTypeEnum.DB2_DATABASE.getType());
        targetObject.setParentUuid(UUID.randomUUID().toString());
        Map<String, String> objectExtendInfo = new HashMap<>();
        objectExtendInfo.put(Db2Constants.NODE_DATABASE_KEY, "testdb");
        targetObject.setExtendInfo(objectExtendInfo);
        task.setTargetObject(targetObject);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.DPF.getType());
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(extendInfo);
        task.setTargetEnv(taskEnvironment);
        task.setTargetLocation(RestoreLocationEnum.NEW);
        return task;
    }

    private Copy mockCopy(String subType) {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        JSONObject resourceJson = new JSONObject();
        resourceJson.put(DatabaseConstants.VERSION, DB2VERSION);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.HADR.getType());
        resourceJson.put(DatabaseConstants.EXTEND_INFO, extendInfo);
        copy.setResourceProperties(JSONObject.fromObject(resourceJson).toString());
        copy.setResourceSubType(subType);
        return copy;
    }

    private ProtectedResource mockDatabaseResource() {
        ProtectedResource databaseResource = new ProtectedResource();
        databaseResource.setVersion(DB2VERSION);
        databaseResource.setExtendInfoByKey("catalogIp", "127.0.0.1");
        return databaseResource;
    }

    private Set<String> mockLockTablespace() {
        Set<String> tablespace = new HashSet<>();
        tablespace.add(UUID.randomUUID().toString());
        return tablespace;
    }
}