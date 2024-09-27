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

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import lombok.extern.slf4j.Slf4j;

import java.util.ArrayList;
import java.util.List;

/**
 * 备份恢复任务工具类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-30
 */
@Slf4j
public final class ProtectionTaskUtils {
    private ProtectionTaskUtils() {
    }

    /**
     * 设置恢复模式
     *
     * @param restoreTask 恢复任务对象
     * @param copyRestApi 副本接口api
     */
    public static void setRestoreMode(RestoreTask restoreTask, CopyRestApi copyRestApi) {
        Copy copy = copyRestApi.queryCopyByID(restoreTask.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            restoreTask.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            restoreTask.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("{} set restore mode success, target uuid: {}, copy id: {}, mode: {}, taskId: {}",
            copy.getResourceSubType(), restoreTask.getTargetObject().getUuid(), restoreTask.getCopyId(),
            restoreTask.getRestoreMode(), restoreTask.getTaskId());
    }

    /**
     * 原位置恢复被保护资源加锁
     *
     * @param task 恢复任务对象
     * @return 需要加锁的资源
     */
    public static List<LockResourceBo> getLockResources(RestoreTask task) {
        List<LockResourceBo> lockResourcesList = new ArrayList<>();
        lockResourcesList.add(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
        return lockResourcesList;
    }

    /**
     * 设置副本格式
     *
     * @param backupTask 备份对象
     */
    public static void setCopyFormat(BackupTask backupTask) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        } else {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        }
    }
}