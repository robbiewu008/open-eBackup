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

import com.huawei.oceanprotect.exercise.service.ExerciseQueryService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.dto.JobExerciseDetail;
import com.huawei.oceanprotect.job.dto.JobLiveMountPolicyDetail;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

/**
 * UnifiedLiveMountJobProvider
 *
 */
@Slf4j
@Component("unifiedLiveMountJobProvider")
public class UnifiedLiveMountJobProvider implements JobProvider {
    @Autowired
    @Qualifier("unifiedJobProvider")
    private JobProvider unifiedJobProvider;

    @Autowired
    private PolicyService liveMountPolicyService;

    @Autowired
    private ExerciseQueryService exerciseQueryService;

    @Override
    public boolean applicable(String jobType) {
        return JobTypeEnum.LIVE_MOUNT.getValue().equals(jobType);
    }

    @Override
    public void stopJob(String associativeId) {
        unifiedJobProvider.stopJob(associativeId);
    }

    @Override
    public void fillJobInfo(Job insertJob) {
        if (!VerifyUtil.isEmpty(insertJob.getExerciseId())) {
            JobExtendInfoUtil.fillJobPolicyInfo(insertJob, exerciseQueryService::queryExercise,
                ext -> insertJob.getExerciseId(), JobExerciseDetail.class, null);
            return;
        }

        JobExtendInfoUtil.fillJobPolicyInfo(insertJob, liveMountPolicyService::getPolicy,
            ext -> JobExtendInfoUtil.getExtInfo(ext, JobExtendInfoKeys.LIVE_MOUNT_POLICY_ID),
            JobLiveMountPolicyDetail.class, null);
    }
}
