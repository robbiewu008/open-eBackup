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
package openbackup.goldendb.protection.access.provider;

import com.huawei.oceanprotect.job.sdk.JobQueueProvider;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.curator.shaded.com.google.common.collect.ImmutableSet;
import org.apache.cxf.common.util.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * GoldenDB任务排队规则
 *
 */
@Component
@AllArgsConstructor
@Slf4j
public class GoldenDBJobQueueProvider implements JobQueueProvider {
    private static final int JOB_LIMIT_ONE = -1;

    private static final String RESOURCE_ID = "resource_id";

    private static final String KEY_QUEUE_JOB_TYPE = "goldendb_job_type";

    /**
     * 排队任务类型
     */
    private static final Set<String> ALLOWED_JOB_TYPE = ImmutableSet.of(JobTypeEnum.BACKUP.getValue(),
        JobTypeEnum.RESTORE.getValue(), JobTypeEnum.COPY_DELETE.getValue(), JobTypeEnum.COPY_EXPIRE.getValue());

    @Override
    public boolean applicable(Job object) {
        return (ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.equalsSubType(object.getSourceSubType())
            || ResourceSubTypeEnum.GOLDENDB_CLUSTER.equalsSubType(object.getSourceSubType()))
            && ALLOWED_JOB_TYPE.contains(object.getType());
    }

    @Override
    public List<JobSchedulePolicy> getCustomizedSchedulePolicy(Job job) {
        // 排队范围
        JSONArray queueScope = new JSONArray();
        JSONObject copyType = new JSONObject();

        // 备份、恢复、副本删除参与排序
        JSONArray typeArray = new JSONArray(new String[] {
            JobTypeEnum.RESTORE.getValue(), JobTypeEnum.BACKUP.getValue(), JobTypeEnum.COPY_DELETE.getValue(),
            JobTypeEnum.COPY_EXPIRE.getValue()
        });

        copyType.put(KEY_QUEUE_JOB_TYPE, typeArray);
        queueScope.add(copyType);
        queueScope.add(RESOURCE_ID);

        // 策略列表
        ArrayList<JobSchedulePolicy> jobSchedulePolicies = new ArrayList<>();

        // 排队策略
        JobSchedulePolicy schedulePolicy = new JobSchedulePolicy();
        schedulePolicy.setScopeJobLimit(JOB_LIMIT_ONE);
        schedulePolicy.setStrictScope(false);
        schedulePolicy.setJobType(job.getType());
        schedulePolicy.setScope(queueScope);
        jobSchedulePolicies.add(schedulePolicy);
        setPayLoad(job);
        log.info("goldendb get jobSchedulePolicies: {}", JsonUtil.json(jobSchedulePolicies));
        return jobSchedulePolicies;
    }

    private void setPayLoad(Job job) {
        JobMessage jobMessage = Optional.ofNullable(JSONObject.toBean(job.getMessage(), JobMessage.class))
            .orElse(new JobMessage());
        JSONObject payload = Optional.ofNullable(jobMessage.getPayload()).orElse(new JSONObject());
        payload.put(KEY_QUEUE_JOB_TYPE, job.getType());
        String resourceId = payload.getString(RESOURCE_ID);
        if (StringUtils.isEmpty(resourceId)) {
            payload.put(RESOURCE_ID, job.getSourceId());
        }
        jobMessage.setPayload(payload);
        job.setMessage(JSONObject.writeValueAsString(jobMessage));
    }
}
