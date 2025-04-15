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
package openbackup.data.access.framework.restore.service;

import static org.assertj.core.api.BDDAssertions.thenThrownBy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.data.access.framework.restore.service.RestoreValidateService;
import openbackup.system.base.common.license.LicenseValidateService;
import openbackup.system.base.sdk.license.enums.FunctionEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Test;
import org.mockito.Mockito;

/**
 * RestoreValidateService的单元测试类
 *
 **/
public class RestoreValidateServiceTest {
    private final LicenseValidateService validateService = Mockito.mock(LicenseValidateService.class);
    private final JobService jobService = Mockito.mock(JobService.class);
    private final FunctionSwitchService functionSwitchService = Mockito.mock(FunctionSwitchService.class);
    private final RestoreValidateService restoreValidateService = new RestoreValidateService(validateService,
        functionSwitchService);

    /**
     * 用例名称：验证传入参数正确的情况下，正常执行业务逻辑<br/>
     * 前置条件：无<br/>
     * check点：每个case分支逻辑执行一次<br/>
     */
    @Test
    public void should_invoke_one_times_when_checkLicense_given_correct_type() {
        //Given - resource type
        final String nasFilesystemType = ResourceSubTypeEnum.NAS_FILESYSTEM.getType();
        final String hdfsFilesetType = ResourceSubTypeEnum.HDFS_FILESET.getType();
        final String hbaseBackupSetType = ResourceSubTypeEnum.HBASE_BACKUPSET.getType();

        //When - check license
        restoreValidateService.checkLicense(nasFilesystemType, RestoreTypeEnum.CR);
        restoreValidateService.checkLicense(hdfsFilesetType, RestoreTypeEnum.IR);
        restoreValidateService.checkLicense(hbaseBackupSetType, RestoreTypeEnum.FLR);

        //Then - verify invoke times
        thenThrownBy(() -> restoreValidateService.checkLicense(nasFilesystemType, null)).isInstanceOf(
            NullPointerException.class);
        verify(validateService, times(1)).validate(nasFilesystemType, FunctionEnum.RECOVERY);
        verify(validateService, times(1)).validate(hdfsFilesetType, FunctionEnum.INSTANT_RECOVERY);
        verify(validateService, times(1)).validate(hbaseBackupSetType, FunctionEnum.FINE_GRAINED_RECOVERY);

    }
}