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
package openbackup.sqlserver.resources.access.provider;

import com.huawei.oceanprotect.job.sdk.JobQueueProvider;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
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
 * sqlserver数据库备份增加排队机制
 *
 */
@Slf4j
@AllArgsConstructor
@Component
public class SqlServerJobQueueProvider implements JobQueueProvider {
    private static final Set<String> ALLOWED_JOB_TYPE = ImmutableSet.of(JobTypeEnum.BACKUP.getValue(),
        JobTypeEnum.RESTORE.getValue(), JobTypeEnum.COPY_DELETE.getValue());

    private static final Integer SING_POD_TASK_NUM = 2;

    private static final String AGENT_ID = "agent_id";

    private static final String SQL_SERVER_JOB_TYPE = "sql_server_queue_job_type";

    private final ResourceService resourceService;

    @Override
    public boolean applicable(Job object) {
        return ResourceSubTypeEnum.SQL_SERVER_DATABASE.equalsSubType(object.getSourceSubType())
            && ALLOWED_JOB_TYPE.contains(object.getType());
    }

    @Override
    public List<JobSchedulePolicy> getCustomizedSchedulePolicy(Job job) {
        log.info("SqlServer Database customize schedulePolicy");
        JobSchedulePolicy jobSchedulePolicy = new JobSchedulePolicy();
        JSONArray jsonArray = new JSONArray();
        JSONObject scope = new JSONObject();
        JSONArray typeArray = new JSONArray(new String[] {
            JobTypeEnum.BACKUP.getValue(), JobTypeEnum.RESTORE.getValue(), JobTypeEnum.COPY_DELETE.getValue()
        });
        scope.put(SQL_SERVER_JOB_TYPE, typeArray);
        jsonArray.add(scope);
        jsonArray.add(AGENT_ID);
        jobSchedulePolicy.setScope(jsonArray);
        jobSchedulePolicy.setScopeJobLimit(SING_POD_TASK_NUM);
        jobSchedulePolicy.setStrictScope(false);
        jobSchedulePolicy.setJobType(job.getType());
        setPayLoad(job);
        return Collections.singletonList(jobSchedulePolicy);
    }

    private void setPayLoad(Job job) {
        JobMessage jobMessage = Optional.ofNullable(JSONObject.toBean(job.getMessage(), JobMessage.class))
            .orElse(new JobMessage());
        JSONObject payload = Optional.ofNullable(jobMessage.getPayload()).orElse(new JSONObject());
        payload.put(SQL_SERVER_JOB_TYPE, job.getType());
        String agentId = payload.getString(AGENT_ID);
        if (StringUtils.isEmpty(agentId)) {
            String resourceUuid = job.getSourceId();
            Optional<ProtectedResource> resourceOptional = resourceService.getBasicResourceById(resourceUuid);
            resourceOptional.ifPresent(protectedResource -> payload.put(AGENT_ID, protectedResource.getRootUuid()));
        }
        jobMessage.setPayload(payload);
        job.setMessage(JSONObject.writeValueAsString(jobMessage));
    }
}
