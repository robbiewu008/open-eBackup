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
package openbackup.data.access.framework.protection.service.job;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

/**
 * UnifiedVerifyJobProvider
 *
 */
@Slf4j
@Component("UnifiedVerifyJobProvider")
public class UnifiedVerifyJobProvider implements JobProvider {
    @Autowired
    @Qualifier("unifiedJobProvider")
    private JobProvider unifiedJobProvider;

    @Autowired
    private CopyRestApi copyRestApi;

    @Override
    public boolean applicable(String jobType) {
        return JobTypeEnum.COPY_VERIFY.getValue().equals(jobType);
    }

    @Override
    public void stopJob(String associativeId) {
        unifiedJobProvider.stopJob(associativeId);
    }

    @Override
    public void fillJobInfo(Job insertJob) {
        fillStorageUnit(insertJob);
    }

    private void fillStorageUnit(Job insertJob) {
        Copy copy = copyRestApi.queryCopyByID(insertJob.getCopyId());
        insertJob.setStorageUnitId(copy.getStorageUnitId());
    }
}
