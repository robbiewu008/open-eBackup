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
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * 公用的任务util
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-22
 */
public class TaskUtilTest {
    /**
     * 用例名称：设置统计速率值到恢复任务扩展参数
     * 前置条件：恢复任务存在
     * 检查点：恢复任务扩展中速率值为期望值
     */
    @Test
    public void set_restore_task_speed_statistics_enum_success() {
        RestoreTask restoreTask = new RestoreTask();
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(restoreTask, SpeedStatisticsEnum.UBC);
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(),
            restoreTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS));
    }

    /**
     * 用例名称：设置统计速率值到备份任务扩展参数
     * 前置条件：备份任务存在
     * 检查点：备份任务扩展中速率值为期望值
     */
    @Test
    public void set_backup_task_speed_statistics_enum_success() {
        BackupTask backupTask = new BackupTask();
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(),
            backupTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS));
    }
}
