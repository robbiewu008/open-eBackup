/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.exchange.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import com.huawei.oceanprotect.job.sdk.JobQueueProvider;

import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.apache.curator.shaded.com.google.common.collect.ImmutableSet;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * Exchange邮箱备份/恢复排队机制
 *
 * @author j00619968
 * @since 2024-04-08
 */
@Slf4j
@AllArgsConstructor
@Component
public class ExchangeMailboxJobQueueProvider implements JobQueueProvider {
    private static final Set<String> ALLOWED_JOB_TYPE = ImmutableSet.of(JobTypeEnum.BACKUP.getValue(),
        JobTypeEnum.RESTORE.getValue());

    private static final Set<String> ALLOWED_EXCHANGE_RESOURCE_TYPE = ImmutableSet.of(
        ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType(), ResourceSubTypeEnum.EXCHANGE_DATABASE.getType());

    private static final String AGENT_ID = "agent_id";

    private static final String EXCHANGE_MAILBOX_JOB_TYPE = "exchange_mailbox_queue_job_type";

    private final ResourceService resourceService;

    @Override
    public boolean applicable(Job object) {
        return ALLOWED_EXCHANGE_RESOURCE_TYPE.contains(object.getSourceSubType())
            && ALLOWED_JOB_TYPE.contains(object.getType());
    }

    @Override
    public List<JobSchedulePolicy> getCustomizedSchedulePolicy(Job job) {
        log.info("Exchange mailbox customize schedulePolicy");
        JobSchedulePolicy jobSchedulePolicy = new JobSchedulePolicy();
        JSONArray jsonArray = new JSONArray();
        JSONObject scope = new JSONObject();
        JSONArray typeArray = new JSONArray(new String[] {
            JobTypeEnum.BACKUP.getValue(), JobTypeEnum.RESTORE.getValue()
        });
        scope.put(EXCHANGE_MAILBOX_JOB_TYPE, typeArray);
        jsonArray.add(scope);
        jsonArray.add(AGENT_ID);
        jobSchedulePolicy.setScope(jsonArray);
        setPayLoadAndMaxConcurrentJobNumber(job, jobSchedulePolicy);
        jobSchedulePolicy.setStrictScope(false);
        jobSchedulePolicy.setJobType(job.getType());
        return Collections.singletonList(jobSchedulePolicy);
    }

    private int getMaxConcurrentJobNumber(String resourceId) {
        int maxConcurrentJobNumber = 1;
        Optional<ProtectedResource> resource = resourceService.getBasicResourceById(resourceId);
        if (resource.isPresent()) {
            maxConcurrentJobNumber = Integer.parseInt(
                resource.get().getExtendInfo().getOrDefault(ExchangeConstant.MAX_CONCURRENT_JOB_NUMBER, "1"));
        }
        log.info("exchange root resource is present: {}, id: {}, maxConcurrentJobNumber: {}", resource.isPresent(),
            resourceId, maxConcurrentJobNumber);
        return maxConcurrentJobNumber;
    }

    private void setPayLoadAndMaxConcurrentJobNumber(Job job, JobSchedulePolicy jobSchedulePolicy) {
        JobMessage jobMessage = Optional.ofNullable(JSONObject.toBean(job.getMessage(), JobMessage.class))
            .orElse(new JobMessage());
        JSONObject payload = Optional.ofNullable(jobMessage.getPayload()).orElse(new JSONObject());
        payload.put(EXCHANGE_MAILBOX_JOB_TYPE, job.getType());
        String agentId = payload.getString(AGENT_ID);
        if (StringUtils.isEmpty(agentId)) {
            String resourceUuid = job.getSourceId();
            Optional<ProtectedResource> resourceOptional = resourceService.getBasicResourceById(resourceUuid);
            resourceOptional.ifPresent(protectedResource -> payload.put(AGENT_ID, protectedResource.getRootUuid()));
        }
        // 设置最大并发数，取负数，设置为全局的最大任务数，正数是单控的限制
        jobSchedulePolicy.setScopeJobLimit(-getMaxConcurrentJobNumber(payload.getString(AGENT_ID)));

        jobMessage.setPayload(payload);
        job.setMessage(JSONObject.writeValueAsString(jobMessage));
    }
}