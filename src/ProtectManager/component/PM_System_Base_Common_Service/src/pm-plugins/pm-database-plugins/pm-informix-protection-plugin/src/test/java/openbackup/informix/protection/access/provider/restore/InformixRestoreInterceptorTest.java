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
package openbackup.informix.protection.access.provider.restore;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.List;

import static org.mockito.ArgumentMatchers.any;

/**
 * InformixRestoreInterceptorTest
 *
 */
public class InformixRestoreInterceptorTest {
    private static final InformixService informixService = PowerMockito.mock(InformixService.class);

    private static final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private static final InformixRestoreInterceptor provider =
            new InformixRestoreInterceptor(copyRestApi, informixService);

    /**
     * 用例场景：applicable集群实例类型
     * 前置条件：类型为Informix-clusterInstance/Informix-singleInstance
     * 检查点：是否返回true
     */
    @Test
    public void applicable_informix_intercept_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType()));
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType()));
    }

    /**
     * 用例场景：恢复数据完善
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    public void intercept_success() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("uuid");
        taskResource.setSubType("1");
        restoreTask.setTargetObject(taskResource);
        restoreTask.setTargetLocation(RestoreLocationEnum.NATIVE);
        restoreTask.setCopyId("copyId");
        restoreTask.setRestoreMode("LocalRestore");
        restoreTask.setTaskId("taskId");
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        restoreTask.setTargetEnv(taskEnvironment);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setVersion("version");
        Mockito.when(informixService.getResourceById(any())).thenReturn(protectedResource);
        Copy copy = new Copy();
        copy.setResourceSubType("Informix-singleInstance");
        copy.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        provider.initialize(restoreTask);
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(), restoreTask.getAdvanceParams().get("speedStatistics"));
        Assert.assertEquals(
                restoreTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE), DatabaseDeployTypeEnum.AP.getType());
    }

    /**
     * 用例场景：获取lock resources
     * 前置条件：无
     * 检查点：是否成功获取lock resources
     */
    @Test
    public void get_lock_resources_success() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("uuid");
        restoreTask.setTargetObject(taskResource);
        List<LockResourceBo> lockResources = provider.getLockResources(restoreTask);
        Assert.assertEquals(1, lockResources.size());
        Assert.assertEquals("uuid", lockResources.get(0).getId());
    }
}
