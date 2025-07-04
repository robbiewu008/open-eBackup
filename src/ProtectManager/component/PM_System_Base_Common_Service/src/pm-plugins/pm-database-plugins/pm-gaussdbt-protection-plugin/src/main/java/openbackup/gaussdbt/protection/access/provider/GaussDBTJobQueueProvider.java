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
package openbackup.gaussdbt.protection.access.provider;

import com.huawei.oceanprotect.job.constants.JobPayloadKeys;
import com.huawei.oceanprotect.job.sdk.JobQueueProvider;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.lang3.StringUtils;
import org.apache.curator.shaded.com.google.common.collect.ImmutableSet;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * GaussDBTJobQueueProvider
 *
 */
@Slf4j
@AllArgsConstructor
@Component
public class GaussDBTJobQueueProvider implements JobQueueProvider {
    private static final Set<String> ALLOWED_JOB_TYPE = ImmutableSet.of(JobTypeEnum.BACKUP.getValue(),
        JobTypeEnum.RESTORE.getValue(), JobTypeEnum.COPY_DELETE.getValue(), JobTypeEnum.COPY_EXPIRE.getValue());

    private final ResourceService resourceService;

    @Override
    public boolean applicable(Job object) {
        return ResourceSubTypeEnum.GAUSSDBT.equalsSubType(object.getSourceSubType()) && ALLOWED_JOB_TYPE.contains(
            object.getType());
    }

    @Override
    public List<JobSchedulePolicy> getCustomizedSchedulePolicy(Job job) {
        log.info("GaussDBT customize schedulePolicy");
        JobSchedulePolicy jobSchedulePolicy = new JobSchedulePolicy();
        JSONArray jsonArray = new JSONArray();
        JSONObject scope = new JSONObject();
        JSONArray typeArray = new JSONArray(new String[] {
            JobTypeEnum.BACKUP.getValue(), JobTypeEnum.RESTORE.getValue(), JobTypeEnum.COPY_DELETE.getValue(),
            JobTypeEnum.COPY_EXPIRE.getValue()
        });
        scope.put(JobPayloadKeys.KEY_QUEUE_JOB_TYPE, typeArray);
        jsonArray.add(scope);
        jsonArray.add(CopyPropertiesKeyConstant.KEY_RESOURCE_PROPERTIES_ROOT_UUID);
        jobSchedulePolicy.setScope(jsonArray);
        jobSchedulePolicy.setScopeJobLimit(-1);
        jobSchedulePolicy.setStrictScope(false);
        jobSchedulePolicy.setJobType(job.getType());
        setPayLoad(job);
        return Collections.singletonList(jobSchedulePolicy);
    }

    private void setPayLoad(Job job) {
        JobMessage jobMessage = Optional.ofNullable(JSONObject.toBean(job.getMessage(), JobMessage.class))
            .orElse(new JobMessage());
        JSONObject payload = Optional.ofNullable(jobMessage.getPayload()).orElse(new JSONObject());
        payload.put(JobPayloadKeys.KEY_QUEUE_JOB_TYPE, job.getType());
        String rootUUid = payload.getString(CopyPropertiesKeyConstant.KEY_RESOURCE_PROPERTIES_ROOT_UUID);
        if (StringUtils.isEmpty(rootUUid)) {
            String resourceUuid = job.getSourceId();
            Optional<ProtectedResource> resourceOptional = resourceService.getBasicResourceById(resourceUuid);
            if (resourceOptional.isPresent()) {
                payload.put(CopyPropertiesKeyConstant.KEY_RESOURCE_PROPERTIES_ROOT_UUID,
                    resourceOptional.get().getRootUuid());
            }
        }
        jobMessage.setPayload(payload);
        job.setMessage(JSONObject.writeValueAsString(jobMessage));
    }
}