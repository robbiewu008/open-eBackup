/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.data.protection.access.provider.sdk.backup;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;

import org.junit.Assert;
import org.junit.Test;

/**
 * BackupTypeConstants测试类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-16
 */
public class BackupTypeConstantsTest {
    /**
     * 用例名称：验证DME的备份类型转为PM的
     * 前置条件：无
     * check点：DME的备份类型成功转为PM
     */
    @Test
    public void dme_backup_type_can_convert_back_success() {
        Assert.assertEquals(BackupTypeConstants.FULL.getBackupType(),
            BackupTypeConstants.dmeBackTypeConvertBack(BackupTypeConstants.DME_BACKUP_FULL));
        Assert.assertEquals(BackupTypeConstants.DIFFERENCE_INCREMENT.getBackupType(),
            BackupTypeConstants.dmeBackTypeConvertBack(BackupTypeConstants.DME_BACKUP_INCREMENT));
        Assert.assertEquals(BackupTypeConstants.CUMULATIVE_INCREMENT.getBackupType(),
            BackupTypeConstants.dmeBackTypeConvertBack(BackupTypeConstants.DME_BACKUP_DIFF));
        Assert.assertEquals(BackupTypeConstants.LOG.getBackupType(),
            BackupTypeConstants.dmeBackTypeConvertBack(BackupTypeConstants.DME_BACKUP_LOG));
        Assert.assertEquals(BackupTypeConstants.PERMANENT_INCREMENT.getBackupType(),
            BackupTypeConstants.dmeBackTypeConvertBack(BackupTypeConstants.DME_BACKUP_FOREVER_INCREMENT));
    }
}
