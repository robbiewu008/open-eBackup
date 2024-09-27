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
package openbackup.tpops.protection.access.provider;

import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.job.sdk.JobQueueProvider;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.apache.curator.shaded.com.google.common.collect.ImmutableSet;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * 功能描述 轻量化排队逻辑
 *
 * @author s30031954
 * @since 2024-01-07
 */
@Component
@AllArgsConstructor
@Slf4j
public class TpopsGaussDbJobQueueProvider implements JobQueueProvider {
    private static final int JOB_LIMIT_ONE = -1;

    private static final String RESOURCE_ID = "resource_id";

    private static final String KEY_QUEUE_JOB_TYPE = "tpops_job_type";

    private static final String RESTORE_TO_LOCAL_POSITION = "Local";

    /**
     * 排队任务类型
     */
    private static final Set<String> ALLOWED_JOB_TYPE = ImmutableSet.of(JobTypeEnum.BACKUP.getValue(),
        JobTypeEnum.RESTORE.getValue(), JobTypeEnum.COPY_DELETE.getValue(), JobTypeEnum.COPY_EXPIRE.getValue());

    private final ResourceService resourceService;

    @Override
    public boolean applicable(Job object) {
        log.info("TpopsGaussDbJobQueueProvider get getSourceSubType: {}", object.getSourceSubType());
        log.info("TpopsGaussDbJobQueueProvider get getType: {}", object.getType());
        return ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.equalsSubType(object.getSourceSubType())
            && ALLOWED_JOB_TYPE.contains(object.getType());
    }

    @Override
    public List<JobSchedulePolicy> getCustomizedSchedulePolicy(Job job) {
        // 排队范围
        JSONArray queueScope = new JSONArray();
        JSONObject copyType = new JSONObject();

        // 备份、恢复、删除副本参与排序
        JSONArray typeArray = new JSONArray(new String[] {
            JobTypeEnum.BACKUP.getValue(), JobTypeEnum.RESTORE.getValue(), JobTypeEnum.COPY_DELETE.getValue(),
            JobTypeEnum.COPY_EXPIRE.getValue()
        });

        // 这个KEY_QUEUE_JOB_TYPE， 每个应用都不要一样。
        copyType.put(KEY_QUEUE_JOB_TYPE, typeArray);
        queueScope.add(copyType);
        queueScope.add(RESOURCE_ID);

        // 策略列表
        ArrayList<JobSchedulePolicy> jobSchedulePolicies = new ArrayList<>();

        // 备份策略
        JobSchedulePolicy backUpPolicy = new JobSchedulePolicy();
        backUpPolicy.setScopeJobLimit(JOB_LIMIT_ONE);
        backUpPolicy.setStrictScope(false);
        backUpPolicy.setJobType(job.getType());
        backUpPolicy.setScope(queueScope);
        jobSchedulePolicies.add(backUpPolicy);
        log.info("Tpops get jobSchedulePolicies: {}", JsonUtil.json(jobSchedulePolicies));
        setPayLoad(job);
        return jobSchedulePolicies;
    }

    private void setPayLoad(Job job) {
        JobMessage jobMessage = Optional.ofNullable(JSONObject.toBean(job.getMessage(), JobMessage.class))
            .orElse(new JobMessage());
        JSONObject payload = Optional.ofNullable(jobMessage.getPayload()).orElse(new JSONObject());
        payload.put(KEY_QUEUE_JOB_TYPE, job.getType());

        // pm传下来的参数，项目ID可能为空，用实例ID的root uuid 作为项目ID
        String projectId = payload.getString(CopyPropertiesKeyConstant.KEY_RESOURCE_PROPERTIES_ROOT_UUID);
        if (StringUtils.isEmpty(projectId)) {
            String resourceUuid = job.getSourceId();
            Optional<ProtectedResource> resourceOptional = resourceService.getBasicResourceById(resourceUuid);
            if (resourceOptional.isPresent()) {
                projectId = resourceOptional.get().getRootUuid();
            }
        }
        log.info("TpopsGaussDbJobQueueProvider targetLocation {}", job.getTargetLocation());

        // 用项目ID + 实例名 作为排队key值
        payload.put(RESOURCE_ID, job.getSourceId());
        if (!StringUtils.equals(RESTORE_TO_LOCAL_POSITION, job.getTargetLocation())) {
            log.info("TpopsGaussDbJobQueueProvider getCustomizedSchedulePolicy targetObject {}", job.getTargetName());
            HashMap<String, Object> conditions = new HashMap<>();
            conditions.put(DatabaseConstants.PARENT_UUID, projectId);
            conditions.put(DatabaseConstants.NAME, job.getTargetName());
            PageListResponse<ProtectedResource> result = resourceService
                .query(true, 0, 1, conditions);
            String instanceId = job.getSourceId();
            if (result.getTotalCount() != 0) {
                instanceId = result.getRecords().get(0).getUuid();
            }
            log.info("TpopsGaussDbJobQueueProvider getCustomizedSchedulePolicy target instance: {}", instanceId);
            payload.put(RESOURCE_ID, instanceId);
        }
        log.info("TpopsGaussDbJobQueueProvider get resourceId: {}", payload.get(RESOURCE_ID));
        jobMessage.setPayload(payload);
        job.setMessage(JSONObject.writeValueAsString(jobMessage));
    }
}