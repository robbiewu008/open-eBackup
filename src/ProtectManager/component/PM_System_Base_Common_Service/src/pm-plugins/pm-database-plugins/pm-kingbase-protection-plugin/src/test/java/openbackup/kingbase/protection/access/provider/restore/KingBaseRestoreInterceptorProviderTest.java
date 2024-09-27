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
package openbackup.kingbase.protection.access.provider.restore;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.kingbase.protection.access.provider.common.KingbaseErrorCode;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link KingBaseRestoreInterceptorProvider}测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-21
 */
public class KingBaseRestoreInterceptorProviderTest {
    private static final String kingBaseVersion = "V008R006C005B0054";

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final KingBaseService kingBaseService = PowerMockito.mock(KingBaseService.class);

    private final KingBaseRestoreInterceptorProvider provider = new KingBaseRestoreInterceptorProvider(kingBaseService,
        copyRestApi);

    /**
     * 用例场景：KingBase恢复适配器
     * 前置条件：输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_kingbase_restore_interceptor_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：获取锁资源
     * 前置条件：无
     * 检查点：是否拿到需要加锁的资源
     */
    @Test
    public void get_lock_resources_success() {
        List<LockResourceBo> lockResourcesList = provider.getLockResources(mockRestoreTask());
        Assert.assertEquals(IsmNumberConstant.ONE, lockResourcesList.size());
    }

    /**
     * 用例场景：kingbse恢复拦截方法设置参数
     * 前置条件：调用到该拦截方法
     * 检查点：参数是否设置正确
     */
    @Test
    public void execute_kingbase_restore_intercept_success() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockCopy());
        PowerMockito.when(kingBaseService.getResourceById(any())).thenReturn(mockInstanceResource());
        PowerMockito.when(kingBaseService.getDeployType(eq(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType())))
            .thenReturn(DatabaseDeployTypeEnum.SINGLE.getType());
        RestoreTask restoreTask = provider.initialize(mockRestoreTask());
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), restoreTask.getRestoreMode());
        Assert.assertEquals(kingBaseVersion,
            restoreTask.getTargetObject().getExtendInfo().get(DatabaseConstants.VERSION));
        Assert.assertEquals(kingBaseVersion,
            restoreTask.getAdvanceParams().get(DatabaseConstants.COPY_PROTECT_OBJECT_VERSION_KEY));
        Assert.assertEquals(RestoreLocationEnum.ORIGINAL.getLocation(),
            restoreTask.getAdvanceParams().get(DatabaseConstants.TARGET_LOCATION_KEY));
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            restoreTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：目标位置与新位置操作系统用户名不一致，不允许下发恢复任务
     * 前置条件：调用到该拦截方法
     * 检查点：目标位置与新位置操作系统用户名是否一致
     */
    @Test
    public void should_raise_LegoCheckedException_when_execute_restore_intercept_failed_if_restore_to_different_os_usr() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockDifUserCopy());
        PowerMockito.when(kingBaseService.getResourceById(any())).thenReturn(mockInstanceResource());
        PowerMockito.when(kingBaseService.getDeployType(eq(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType())))
            .thenReturn(DatabaseDeployTypeEnum.SINGLE.getType());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> provider.initialize(mockRestoreTask()));
        Assert.assertEquals(KingbaseErrorCode.OS_USER_NOT_EQUAL_BEFORE_RESTORE, legoCheckedException.getErrorCode());

    }

    /**
     * 用例场景：目标位置与新位置数据库模式不一致，不允许下发恢复任务
     * 前置条件：调用到该拦截方法
     * 检查点：目标位置与新位置数据库模式是否一致
     */
    @Test
    public void should_raise_LegoCheckedException_if_restore_to_different_db_mode_when_execute_restore_intercept() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockDifDbModeCopy());
        ProtectedResource instanceResource = mockInstanceResource();
        instanceResource.setExtendInfoByKey(DatabaseConstants.DB_MODE_KEY, "oracle");
        PowerMockito.when(kingBaseService.getResourceById(any())).thenReturn(instanceResource);
        PowerMockito.when(kingBaseService.getDeployType(eq(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType())))
            .thenReturn(DatabaseDeployTypeEnum.SINGLE.getType());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> provider.initialize(mockRestoreTask()));
        Assert.assertEquals(DatabaseErrorCode.RESTORE_RESOURCE_VERSION_INCONSISTENT,
            legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：副本没有数据库模式信息，允许下发恢复任务
     * 前置条件：调用到该拦截方法
     * 检查点：正常下发，无异常抛出
     */
    @Test
    public void should_execute_success_if_restore_copy_db_mode_is_empty_when_execute_restore_intercept() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockCopy());
        PowerMockito.when(kingBaseService.getResourceById(any())).thenReturn(mockInstanceResource());
        PowerMockito.when(kingBaseService.getDeployType(eq(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType())))
            .thenReturn(DatabaseDeployTypeEnum.SINGLE.getType());
        provider.initialize(mockRestoreTask());
        Assert.assertTrue(true);
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid(UUID.randomUUID().toString());
        targetObject.setSubType(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        restoreTask.setTargetObject(targetObject);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        restoreTask.setTargetEnv(taskEnvironment);
        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        return restoreTask;
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        JSONObject resourceJson = new JSONObject();
        resourceJson.put(DatabaseConstants.VERSION, kingBaseVersion);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("osUsername", "kingbase");
        resourceJson.put(DatabaseConstants.EXTEND_INFO, extendInfo);
        copy.setResourceProperties(JSONObject.fromObject(resourceJson).toString());
        return copy;
    }

    private Copy mockDifUserCopy() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        JSONObject resourceJson = new JSONObject();
        resourceJson.put(DatabaseConstants.VERSION, kingBaseVersion);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("osUsername", "kingdase");
        resourceJson.put(DatabaseConstants.EXTEND_INFO, extendInfo);
        copy.setResourceProperties(JSONObject.fromObject(resourceJson).toString());
        return copy;
    }

    private ProtectedResource mockInstanceResource() {
        ProtectedResource instanceResource = new ProtectedResource();
        instanceResource.setVersion(kingBaseVersion);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("osUsername", "kingbase");
        instanceResource.setExtendInfo(extendInfo);
        return instanceResource;
    }

    private Copy mockDifDbModeCopy() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        JSONObject resourceJson = new JSONObject();
        resourceJson.put(DatabaseConstants.VERSION, kingBaseVersion);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("osUsername", "kingbase");
        extendInfo.put(DatabaseConstants.DB_MODE_KEY, "pg");
        resourceJson.put(DatabaseConstants.EXTEND_INFO, extendInfo);
        copy.setResourceProperties(JSONObject.fromObject(resourceJson).toString());
        return copy;
    }
}