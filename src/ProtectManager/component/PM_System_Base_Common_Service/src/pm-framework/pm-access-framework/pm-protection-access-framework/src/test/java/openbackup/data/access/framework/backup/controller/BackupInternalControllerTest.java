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
package openbackup.data.access.framework.backup.controller;

import openbackup.data.access.framework.backup.controller.BackupInternalController;
import openbackup.data.access.framework.backup.controller.vo.TransferBackupVo;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupFeatureService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * BackupInternalController测试类
 *
 */
@RunWith(SpringRunner.class)
@PrepareForTest(BackupInternalController.class)
@SpringBootTest(classes = BackupInternalController.class)
public class BackupInternalControllerTest {
    @Autowired
    private BackupInternalController backupInternalController;

    @MockBean
    private BackupFeatureService backupFeatureService;

    /**
     * 查询支持并行备份接口：成功返回是否支持并行备份
     */
    @Test
    public void check_success_parallel_backup_settings() {
        Mockito.when(backupFeatureService.isSupportDataAndLogParallelBackup("11")).thenReturn(true);
        boolean res = backupInternalController.isSupportDataAndLogParallelBackup("11");
        Assert.assertTrue(res);
    }

    /**
     * 查询转换备份类型接口：参数错误抛出异常
     */
    @Test
    public void check_throw_exception_when_params_error() {
        Mockito.when(backupFeatureService.transferBackupType(Mockito.any(), Mockito.anyString()))
            .thenReturn(BackupTypeConstants.FULL);

        TransferBackupVo transferBackupVo = new TransferBackupVo();
        transferBackupVo.setBackupType("aaa");
        transferBackupVo.setResourceId("11");

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> backupInternalController.transferBackupType(transferBackupVo));
        Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.ERR_PARAM);
    }

    /**
     * 查询转换备份类型接口：成功返回转换后类型
     */
    @Test
    public void check_success_transfer_backup_type() {
        Mockito.when(backupFeatureService.transferBackupType(Mockito.any(), Mockito.anyString()))
            .thenReturn(BackupTypeConstants.FULL);

        TransferBackupVo transferBackupVo = new TransferBackupVo();
        transferBackupVo.setBackupType("log");
        transferBackupVo.setResourceId("11");

        BackupTypeConstants backupTypeConstants = backupInternalController.transferBackupType(transferBackupVo);
        Assert.assertEquals(backupTypeConstants, BackupTypeConstants.FULL);
    }
}
