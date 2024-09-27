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
package openbackup.gaussdbt.protection.access.provider.restore;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.List;

/**
 * {@link GaussDBTSingleRestoreProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/26
 */
public class GaussDBTSingleRestoreProviderTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final GaussDBTSingleService gaussDBTSingleService = PowerMockito.mock(GaussDBTSingleService.class);

    private GaussDBTSingleRestoreProvider provider = new GaussDBTSingleRestoreProvider(copyRestApi,
        gaussDBTSingleService);

    /**
     * 用例场景：框架调applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：GaussDBT-single类型是否返回true，其他类型是否返回false
     */
    @Test
    public void applicable_gaussdbt_single_restore_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
    }

    /**
     * 用例场景：GaussDBT单机恢复到原位置
     * 前置条件：服务正常，调用到插件
     * 检查点：检查参数是否设置正确
     */
    @Test
    public void should_return_params_if_restore_original_when_execute_restore() {
        PowerMockito.when(gaussDBTSingleService.getResourceById(any())).thenReturn(mockResource());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockCopy());
        RestoreTask resultTask = provider.initialize(mockRestoreTask(RestoreLocationEnum.ORIGINAL));
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(),
            resultTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS));
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            resultTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
        Assert.assertEquals("1.2.1", resultTask.getTargetObject().getExtendInfo().get(DatabaseConstants.VERSION));
        Assert.assertEquals(RestoreLocationEnum.ORIGINAL.getLocation(),
            resultTask.getAdvanceParams().get(DatabaseConstants.TARGET_LOCATION_KEY));
    }

    /**
     * 用例场景：GaussDBT单机恢复到新位置
     * 前置条件：服务正常，调用到插件
     * 检查点：检查参数是否设置正确
     */
    @Test
    public void should_return_params_if_restore_new_when_execute_restore() {
        PowerMockito.when(gaussDBTSingleService.getResourceById(any())).thenReturn(mockResource());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockCopy());
        RestoreTask resultTask = provider.initialize(mockRestoreTask(RestoreLocationEnum.NEW));
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(),
            resultTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS));
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            resultTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
        Assert.assertEquals("1.2.1", resultTask.getTargetObject().getExtendInfo().get(DatabaseConstants.VERSION));
        Assert.assertEquals(RestoreLocationEnum.NEW.getLocation(),
            resultTask.getAdvanceParams().get(DatabaseConstants.TARGET_LOCATION_KEY));
    }

    /**
     * 用例场景：GaussDBT单机数据库恢复获取需要锁定的资源
     * 前置条件：资源正常
     * 检查点：是否拿到需要加锁的资源
     */
    @Test
    public void should_return_lock_resource_if_restore_when_getLockResources() {
        List<LockResourceBo> lockResourceBoList = provider.getLockResources(mockRestoreTask(RestoreLocationEnum.NEW));
        Assert.assertEquals(IsmNumberConstant.ONE, lockResourceBoList.size());
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setResourceSubType(ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType());
        return copy;
    }

    private RestoreTask mockRestoreTask(RestoreLocationEnum targetLocation) {
        RestoreTask task = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUIDGenerator.getUUID());
        task.setTargetObject(taskResource);
        task.setCopyId(UUIDGenerator.getUUID());
        task.setTaskId(UUIDGenerator.getUUID());
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        task.setTargetEnv(taskEnvironment);
        task.setTargetLocation(targetLocation);
        return task;
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(UUIDGenerator.getUUID());
        resource.setVersion("1.2.1");
        return resource;
    }
}