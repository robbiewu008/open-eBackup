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
package openbackup.gaussdbt.protection.access.provider.backup;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.MountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;

/**
 * {@link GaussDBTSingleBackupProvider} 测试类
 *
 */
public class GaussDBTSingleBackupProviderTest {
    private GaussDBTSingleService gaussDBTSingleService = PowerMockito.mock(GaussDBTSingleService.class);

    private GaussDBTSingleBackupProvider provider = new GaussDBTSingleBackupProvider(gaussDBTSingleService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：GaussDBT-single类型是否返回true，其他类型是否返回false
     */
    @Test
    public void applicable_gaussdbt_single_backup_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
    }

    /**
     * 用例场景：GaussDBT-single全量备份拦截器设置参数
     * 前置条件：服务正常，正确调用到拦截器
     * 检查点：检查参数是否设置正确
     */
    @Test
    public void should_return_backup_params_if_full_backup_when_execute_intercept() {
        BackupTask backupTask = mockBackupTask(DatabaseConstants.FULL_BACKUP_TYPE);
        BackupTask resultTask = provider.initialize(backupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            resultTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
        Assert.assertEquals(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat(), resultTask.getCopyFormat());
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(), resultTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS));
        Assert.assertEquals(IsmNumberConstant.TWO, resultTask.getRepositories().size());
    }

    /**
     * 用例场景：GaussDBT-single日志备份拦截器设置参数
     * 前置条件：服务正常，正确调用到拦截器
     * 检查点：检查参数是否设置正确
     */
    @Test
    public void should_return_backup_params_if_log_backup_when_execute_intercept() {
        BackupTask backupTask = mockBackupTask(DatabaseConstants.LOG_BACKUP_TYPE);
        BackupTask resultTask = provider.initialize(backupTask);
        Assert.assertEquals(IsmNumberConstant.THREE, resultTask.getRepositories().size());
        Assert.assertEquals(MountTypeEnum.NON_FULL_PATH_MOUNT.getMountType(),
            resultTask.getAdvanceParams().get(GaussDBTConstant.MOUNT_TYPE_KEY));
    }

    private BackupTask mockBackupTask(String backupType) {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        backupTask.setProtectEnv(taskEnvironment);
        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        backupTask.setRepositories(new ArrayList<>(Collections.singletonList(dataRepository)));
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUIDGenerator.getUUID());
        backupTask.setProtectObject(taskResource);
        backupTask.setBackupType(backupType);
        return backupTask;
    }
}