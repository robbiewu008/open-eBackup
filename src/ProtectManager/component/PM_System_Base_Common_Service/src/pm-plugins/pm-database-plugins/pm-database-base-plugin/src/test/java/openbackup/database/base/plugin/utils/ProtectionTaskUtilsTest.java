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
package openbackup.database.base.plugin.utils;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.List;

/**
 * 数据库扫描公共工具测试类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-30
 */
public class ProtectionTaskUtilsTest {
    private CopyRestApi copyRestApi;

    @Before
    public void init() {
        this.copyRestApi = Mockito.mock(CopyRestApi.class);
    }

    /**
     * 用例场景：设置云归档恢复模式成功
     * 前置条件：备份成功
     * 检 查 点：成功设置恢复模式
     */
    @Test
    public void set_restoreMode_by_cloud_archive_success() {
        RestoreTask restoreTask = getRestoreTask(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE);
        ProtectionTaskUtils.setRestoreMode(restoreTask, copyRestApi);
        Assert.assertEquals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：设置磁带归档恢复模式成功
     * 前置条件：备份成功
     * 检 查 点：成功设置恢复模式
     */
    @Test
    public void set_restoreMode_by_tape_archive_success() {
        RestoreTask restoreTask = getRestoreTask(CopyGeneratedByEnum.BY_TAPE_ARCHIVE);
        ProtectionTaskUtils.setRestoreMode(restoreTask, copyRestApi);
        Assert.assertEquals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：设置其他归档恢复模式成功
     * 前置条件：备份成功
     * 检 查 点：成功设置恢复模式
     */
    @Test
    public void set_restoreMode_by_other_archive_success() {
        RestoreTask restoreTask = getRestoreTask(CopyGeneratedByEnum.BY_LIVE_MOUNTE);
        ProtectionTaskUtils.setRestoreMode(restoreTask, copyRestApi);
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：测试原位置恢复资源互斥加锁
     * 前置条件：备份成功，恢复正常走到pm插件
     * 检 查 点：返回原位置恢复加锁资源不为空
     */
    @Test
    public void set_should_locked_resources_in_original_restore_success() {
        RestoreTask restoreTask = getRestoreTask(CopyGeneratedByEnum.BY_TAPE_ARCHIVE);
        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        List<LockResourceBo> lockResources = ProtectionTaskUtils.getLockResources(restoreTask);
        Assert.assertEquals(1, lockResources.size());
    }

    /**
     * 用例场景：根据任务类型设置备份时的副本格式
     * 前置条件：1. 不是日志备份
     * 检 查 点：1. 副本格式是1
     */
    @Test
    public void set_copy_format_success_when_backup_type_is_log() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        ProtectionTaskUtils.setCopyFormat(backupTask);
        Assert.assertTrue(backupTask.getCopyFormat() == CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
    }

    /**
     * 用例场景：根据任务类型设置备份时的副本格式
     * 前置条件：1. 不是日志备份
     * 检 查 点：1. 副本格式是1
     */
    @Test
    public void set_copy_format_success_when_backup_type_is_not_log() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        ProtectionTaskUtils.setCopyFormat(backupTask);
        Assert.assertTrue(backupTask.getCopyFormat() == CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
    }

    private RestoreTask getRestoreTask(CopyGeneratedByEnum copyGeneratedByEnum) {
        Copy copy = new Copy();
        copy.setGeneratedBy(copyGeneratedByEnum.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("222222");
        restoreTask.setTargetObject(taskResource);
        restoreTask.setCopyId("111111");
        return restoreTask;
    }
}
