/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.database.base.plugin.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import com.huawei.oceanprotect.job.constants.JobPayloadKeys;
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

import org.apache.curator.shaded.com.google.common.collect.ImmutableSet;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 通用数据库GeneralDBJobQueueProvider
 *
 * @author t30049904
 * @version [OceanProtect DataBack 1.5.0]
 * @since 2023/12/1
 */
@Slf4j
@AllArgsConstructor
@Component
public class GeneralDbJobQueueProvider implements JobQueueProvider {
    private static final String GBASE_RESOURCE_TYPE = "Gbase 8a";
    private static final Set<String> ALLOWED_JOB_TYPE = ImmutableSet.of(JobTypeEnum.BACKUP.getValue(),
            JobTypeEnum.RESTORE.getValue());

    private final ResourceService resourceService;

    @Override
    public boolean applicable(Job object) {
        return ResourceSubTypeEnum.GENERAL_DB.equalsSubType(object.getSourceSubType())
                && ALLOWED_JOB_TYPE.contains(object.getType());
    }

    @Override
    public List<JobSchedulePolicy> getCustomizedSchedulePolicy(Job job) {
        if (GBASE_RESOURCE_TYPE.equals(getGbaseType(job))) {
            return getGbasePolicy(job);
        }
        return Collections.emptyList();
    }

    private List<JobSchedulePolicy> getGbasePolicy(Job job) {
        JobSchedulePolicy jobSchedulePolicy = new JobSchedulePolicy();
        JSONArray jsonArray = new JSONArray();
        JSONObject scope = new JSONObject();
        JSONArray typeArray = new JSONArray(new String[] {JobTypeEnum.BACKUP.getValue(),
                JobTypeEnum.RESTORE.getValue()});
        scope.put(GeneralDbConstant.EXTEND_RELATED_HOST_IPS, getResourceHostIps(job));
        scope.put(JobPayloadKeys.KEY_QUEUE_JOB_TYPE, typeArray);
        jsonArray.add(scope);
        jobSchedulePolicy.setScope(jsonArray);
        jobSchedulePolicy.setScopeJobLimit(-1);
        jobSchedulePolicy.setStrictScope(false);
        jobSchedulePolicy.setJobType(job.getType());
        setPayLoad(job);
        return Collections.singletonList(jobSchedulePolicy);
    }

    private String getResourceHostIps(Job job) {
        String resourceUuid = job.getSourceId();
        Optional<ProtectedResource> resource = resourceService.getBasicResourceById(resourceUuid);
        String[] ips = resource.map(ProtectedResource::getExtendInfo)
                .map(e -> e.get(GeneralDbConstant.EXTEND_RELATED_HOST_IPS)).map(e -> e.split(","))
                .orElse(new String[0]);
        return Arrays.stream(ips).filter(Objects::nonNull).sorted().collect(Collectors.joining(","));
    }

    private String getGbaseType(Job job) {
        String resourceUuid = job.getSourceId();
        Optional<ProtectedResource> typeResource = resourceService.getBasicResourceById(resourceUuid);
        return typeResource.isPresent()
                ? typeResource.get().getExtendInfo().get(GeneralDbConstant.DATABASE_TYPE_DISPLAY) : "Dont get Type";
    }

    private void setPayLoad(Job job) {
        JobMessage jobMessage = Optional
                                .ofNullable(JSONObject.toBean(job.getMessage(), JobMessage.class))
                                .orElse(new JobMessage());
        JSONObject payload = Optional.ofNullable(jobMessage.getPayload()).orElse(new JSONObject());
        payload.put(GeneralDbConstant.EXTEND_RELATED_HOST_IPS, getResourceHostIps(job));
        payload.put(JobPayloadKeys.KEY_QUEUE_JOB_TYPE, job.getType());
        jobMessage.setPayload(payload);
        job.setMessage(JSONObject.writeValueAsString(jobMessage));
    }
}
