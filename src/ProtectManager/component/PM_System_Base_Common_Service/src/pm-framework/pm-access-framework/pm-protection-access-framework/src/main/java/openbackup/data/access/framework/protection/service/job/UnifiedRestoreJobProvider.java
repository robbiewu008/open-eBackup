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
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.dto.JobExerciseDetail;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * UnifiedRestoreProvider
 *
 */

@Slf4j
@Component("UnifiedRestoreJobProvider")
public class UnifiedRestoreJobProvider implements JobProvider {
    @Autowired
    @Qualifier("unifiedJobProvider")
    private JobProvider unifiedJobProvider;

    @Autowired
    private FunctionSwitchService functionSwitchService;

    @Autowired
    private ExerciseQueryService exerciseQueryService;

    @Autowired
    private ResourceService resourceService;

    @Override
    public boolean applicable(String jobType) {
        return JobTypeEnum.RESTORE.getValue().equals(jobType);
    }

    @Override
    public void stopJob(String associativeId) {
        unifiedJobProvider.stopJob(associativeId);
    }

    @Override
    public void fillJobInfo(Job insertJob) {
        JSONObject jobExtendFiled = JSONObject.fromObject(insertJob.getExtendStr());
        convertAgentsIdToName(jobExtendFiled, insertJob);
        if (!VerifyUtil.isEmpty(insertJob.getExerciseId())) {
            JobExtendInfoUtil.fillJobPolicyInfo(insertJob, exerciseQueryService::queryExercise,
                ext -> insertJob.getExerciseId(), JobExerciseDetail.class, null);
        }
    }

    private void convertAgentsIdToName(JSONObject jobExtendFiled, Job insertJob) {
        if (jobExtendFiled.containsKey(JobExtendInfoKeys.JOB_CONFIG)) {
            JSONObject jobConfig = jobExtendFiled.getJSONObject(JobExtendInfoKeys.JOB_CONFIG);
            if (jobConfig.containsKey(JobExtendInfoKeys.AGENTS)) {
                JSONArray agents = jobConfig.getJSONArray(JobExtendInfoKeys.AGENTS);
                List<String> agentIdList = agents.toBean(String.class);
                List<String> agentNames = agentIdList.stream()
                    .map(agentId -> resourceService.getBasicResourceById(false, agentId)
                        .map(resource -> resource.getName() + "(" + resource.getEndpoint() + ")")
                        .orElse(null))
                    .filter(Objects::nonNull)
                    .collect(Collectors.toList());
                jobConfig.set(JobExtendInfoKeys.AGENTS, agentNames);
                insertJob.setExtendStr(jobExtendFiled.toString());
            }
        }
    }
}
