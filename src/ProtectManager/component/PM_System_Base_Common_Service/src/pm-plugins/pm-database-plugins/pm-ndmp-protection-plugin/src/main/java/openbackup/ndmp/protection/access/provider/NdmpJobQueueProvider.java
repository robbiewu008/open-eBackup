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
package openbackup.ndmp.protection.access.provider;

import com.huawei.oceanprotect.job.sdk.JobQueueProvider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.curator.shaded.com.google.common.collect.ImmutableSet;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * ndmp自定义任务排队规则
 *
 */
@Slf4j
@Component
public class NdmpJobQueueProvider implements JobQueueProvider {
    private static final String NDMP_JOB_TYPE = "ndmp_queue_job_type";

    private static final String NDMP_STORAGE_EQUIPMENT_ID = "ndmp_storage_equipment_id";

    private static final int DEFAULT_MAX_CONCURRENT_JOB_NUMBER = 8;

    private static final Set<String> ALLOWED_JOB_TYPE = ImmutableSet.of(JobTypeEnum.BACKUP.getValue(),
        JobTypeEnum.RESTORE.getValue());

    @Autowired
    private ResourceService resourceService;

    @Override
    public boolean applicable(Job job) {
        return ResourceSubTypeEnum.NDMP_BACKUPSET.getType().equals(job.getSourceSubType()) && ALLOWED_JOB_TYPE.contains(
            job.getType());
    }

    @Override
    public List<JobSchedulePolicy> getCustomizedSchedulePolicy(Job job) {
        log.info("ndmp customize schedulePolicy");
        JobSchedulePolicy jobSchedulePolicy = new JobSchedulePolicy();
        JSONArray jsonArray = new JSONArray();
        JSONObject scope = new JSONObject();
        scope.put(NDMP_JOB_TYPE, ALLOWED_JOB_TYPE);
        jsonArray.add(scope);
        jsonArray.add(NDMP_STORAGE_EQUIPMENT_ID);
        jobSchedulePolicy.setScope(jsonArray);
        setPayLoadAndMaxConcurrentJobNumber(job, jobSchedulePolicy);
        jobSchedulePolicy.setStrictScope(false);
        jobSchedulePolicy.setJobType(job.getType());
        return Collections.singletonList(jobSchedulePolicy);
    }

    private void setPayLoadAndMaxConcurrentJobNumber(Job job, JobSchedulePolicy jobSchedulePolicy) {
        JobMessage jobMessage = Optional.ofNullable(JSONObject.toBean(job.getMessage(), JobMessage.class))
            .orElse(new JobMessage());
        JSONObject payload = Optional.ofNullable(jobMessage.getPayload()).orElse(new JSONObject());
        payload.put(NDMP_JOB_TYPE, job.getType());
        String environmentId = getEnvId(job, payload);
        payload.put(NDMP_STORAGE_EQUIPMENT_ID, environmentId);

        // 设置最大并发数，取负数，设置为全局的最大任务数，正数是单控的限制
        jobSchedulePolicy.setScopeJobLimit(-getMaxConcurrentJobNumber(environmentId));

        jobMessage.setPayload(payload);
        job.setMessage(JSONObject.writeValueAsString(jobMessage));
    }

    private String getEnvId(Job job, JSONObject payload) {
        if (JobTypeEnum.BACKUP.getValue().equals(job.getType())) {
            String resourceId = job.getSourceId();
            ProtectedResource resource = resourceService.getBasicResourceById(resourceId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                    "Protected Resource is not exists. uuid: " + resourceId));
            return resource.getRootUuid();
        } else {
            return payload.getJSONObject(NdmpConstant.TARGET_ENV).getString(NdmpConstant.UUID);
        }
    }

    private int getMaxConcurrentJobNumber(String resourceId) {
        // 任务并发数默认为8
        int maxConcurrentJobNumber = DEFAULT_MAX_CONCURRENT_JOB_NUMBER;
        Optional<ProtectedResource> resource = resourceService.getBasicResourceById(resourceId);
        if (resource.isPresent()) {
            maxConcurrentJobNumber = Integer.parseInt(resource.get()
                .getExtendInfo()
                .getOrDefault(NdmpConstant.MAX_CONCURRENT_JOB_NUMBER,
                    String.valueOf(DEFAULT_MAX_CONCURRENT_JOB_NUMBER)));
        }
        log.info("ndmp storage equipment is present: {}, id: {}, maxConcurrentJobNumber: {}", resource.isPresent(),
            resourceId, maxConcurrentJobNumber);
        return maxConcurrentJobNumber;
    }
}
