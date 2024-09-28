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
package openbackup.data.protection.access.provider.sdk.util;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * 公用的任务util
 *
 */
public class TaskUtil {
    /**
     * 速率统计 1为ubc统计，2为应用统计
     */
    public static final String SPEED_STATISTICS = "speedStatistics";

    /**
     * 往备份任务的高级参数里添加速度统计方式
     *
     * @param backupTask 备份对象
     * @param speedStatistics 速度统计方式
     */
    public static void setBackupTaskSpeedStatisticsEnum(BackupTask backupTask, SpeedStatisticsEnum speedStatistics) {
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(SPEED_STATISTICS, speedStatistics.getType());
        backupTask.setAdvanceParams(advanceParams);
    }

    /**
     * 往恢复任务的高级参数里添加速度统计方式
     *
     * @param restoreTask 恢复对象
     * @param speedStatistics 速度统计方式
     */
    public static void setRestoreTaskSpeedStatisticsEnum(RestoreTask restoreTask, SpeedStatisticsEnum speedStatistics) {
        Map<String, String> advanceParams = Optional.ofNullable(restoreTask.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(SPEED_STATISTICS, speedStatistics.getType());
        restoreTask.setAdvanceParams(advanceParams);
    }
}
