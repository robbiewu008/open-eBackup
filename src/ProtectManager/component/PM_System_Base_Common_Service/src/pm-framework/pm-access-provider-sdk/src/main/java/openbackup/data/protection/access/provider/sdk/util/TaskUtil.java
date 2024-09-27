/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-22
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
