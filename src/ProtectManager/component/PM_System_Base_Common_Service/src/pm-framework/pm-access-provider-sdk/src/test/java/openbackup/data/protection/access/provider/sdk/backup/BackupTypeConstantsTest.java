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
package openbackup.data.protection.access.provider.sdk.backup;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;

import org.junit.Assert;
import org.junit.Test;

/**
 * BackupTypeConstants测试类
 *
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
