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
package openbackup.data.access.framework.protection.service;

import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.report.enums.ProtectStatusEnum;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.ScheduleDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.enums.Trigger;
import com.huawei.oceanprotect.sla.sdk.enums.TriggerAction;
import com.huawei.oceanprotect.system.base.schedule.common.enums.ScheduleType;
import com.huawei.oceanprotect.system.base.schedule.service.SchedulerService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.alibaba.fastjson.JSON;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ResourceGroupRepository;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.access.framework.core.entity.ProtectedTaskPo;
import openbackup.data.protection.access.provider.sdk.constants.ProtectObjectErrorCode;
import openbackup.data.protection.access.provider.sdk.protection.model.ProtectionExecuteDto;
import openbackup.data.protection.access.provider.sdk.protection.model.ProtectionResourceDto;
import openbackup.data.protection.access.provider.sdk.protection.service.ProjectedObjectService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.enums.GroupTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.ResourceErrorCodeConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.ProtectionJobStepEnum;
import openbackup.system.base.common.enums.ProtectionModifyJobStepEnum;
import openbackup.system.base.common.enums.WeekDayEnum;
import openbackup.system.base.common.enums.WormValidityTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.thread.ThreadPoolTool;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.job.model.request.JobStepParam;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.model.Schedule;

import org.quartz.SchedulerException;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * 保护服务实现
 *
 */
@Component
@Slf4j
public class ProjectedObjectServiceImpl implements ProjectedObjectService {
    private static final String GROUP_BACKUP_TOPIC = "schedule.group_backup";

    private static final String AUTOMATIC = "AUTOMATIC";

    private static final UUID NAMESPACE_OID = UUID.fromString("6ba7b810-9dad-11d1-80b4-00c04fd430c8");

    private static final String SCAN_VM_UNDER_COMPUTE_RES = "ScanVmUnderComputeResource";

    private static final String VHD_SET = "VHDSet";

    private static final Set<String> VMWARE_TYPES = new HashSet() {{
        add(ResourceSubTypeEnum.VMWARE.getType());
        add(ResourceSubTypeEnum.HOST_SYSTEM.getType());
        add(ResourceSubTypeEnum.CLUSTER_COMPUTE_RESOURCE.getType());
    }};

    @Autowired
    private JobService jobService;

    @Autowired
    private SlaQueryService slaQueryService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Autowired
    private ProtectedObjectMapper protectedObjectMapper;

    @Autowired
    private SchedulerService scheduleService;

    @Autowired
    private ProjectedTaskService projectedTaskService;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ResourceGroupRepository resourceGroupRepository;

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Override
    public String createGroupProjectedObject(ProtectionExecuteDto dto, Consumer<String> postAction) {
        /**
         * 1.查询sla
         * 2.创建一个任务
         * 3.更新保护状态为创建中
         * 4.填充保护对象的扩展字段
         */
        SlaDto sla = slaQueryService.querySlaById(dto.getSlaId());
        TokenBo.UserBo user = TokenBo.get().getUser();
        CreateJobRequest job = buildJobReq(dto.getResource(), user, sla, JobTypeEnum.RESOURCE_PROTECTION);
        String jobId = jobService.createJob(job);
        dto.setJobId(jobId);
        dto.setUserId(user.getId());
        fillParam(dto, sla);
        ThreadPoolTool.getPool().execute(() -> {
            if (doCreateGroupProjectedObject(dto, sla, jobId)) {
                postAction.accept(user.getId());
            }
        });
        return jobId;
    }

    @Override
    public String changeGroupProjectedObject(ProtectionExecuteDto dto, List<String> toDeleteResourceIds) {
        SlaDto sla = slaQueryService.querySlaById(dto.getSlaId());
        TokenBo.UserBo user = TokenBo.get().getUser();
        CreateJobRequest job = buildJobReq(dto.getResource(), user, sla, JobTypeEnum.RESOURCE_PROTECTION_MODIFY);
        String jobId = jobService.createJob(job);
        dto.setJobId(jobId);
        dto.setUserId(user.getId());
        fillParam(dto, sla);
        ThreadPoolTool.getPool().execute(() -> {
            doUpdateGroupProjectedObject(dto, sla, jobId, toDeleteResourceIds);
        });
        return jobId;
    }

    private void doUpdateGroupProjectedObject(ProtectionExecuteDto dto, SlaDto sla, String jobId,
        List<String> toDeleteResourceIds) {
        recordJobStep(JobStepParam.builder().jobId(jobId)
            .logInfo(ProtectionModifyJobStepEnum.PROTECTION_MODIFY_START.getValue())
            .level(JobLogLevelEnum.INFO.getValue())
            .status(JobStatusEnum.RUNNING)
            .build());
        List<String> failedList = new ArrayList<>();
        List<String> successList = new ArrayList<>();
        removeProtectedByIds(toDeleteResourceIds);
        for (ProtectionResourceDto resource : dto.getSubResources()) {
            boolean subResult = protectSubResource(dto, sla, jobId, resource);
            if (subResult) {
                successList.add(resource.getUuid());
            } else {
                failedList.add(resource.getUuid());
            }
        }
        // 保持和资源组一样的保护，可能是创建保护后禁用状态
        ResourceGroupDto group = dto.getResource();
        int successStatus = VerifyUtil.isEmpty(group.getProtectedObjectDto())
            ? ProtectStatusEnum.PROTECTED.getProtectStatusNum() : group.getProtectedObjectDto().getStatus();
        resourceService.batchUpdateStatusById(successList, successStatus);
        resourceService.batchUpdateStatusById(failedList, ProtectStatusEnum.UNPROTECTED.getProtectStatusNum());
        if (VerifyUtil.isEmpty(failedList)) {
            // 成功
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionModifyJobStepEnum.PROTECTION_MODIFY_FINISH.getValue())
                .level(JobLogLevelEnum.INFO.getValue())
                .status(JobStatusEnum.SUCCESS)
                .build());
        } else {
            // 失败
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionModifyJobStepEnum.PROTECTION_MODIFY_FAILED.getValue())
                .level(JobLogLevelEnum.FATAL.getValue())
                .status(JobStatusEnum.FAIL)
                .build());
        }
    }

    private boolean doCreateGroupProjectedObject(ProtectionExecuteDto dto, SlaDto sla, String jobId) {
        /**
         * 5.创建保护对象及调度任务
         * 6.保护对象入库
         * 7.更新资源表中的保护状态为已保护
         */
        recordJobStep(JobStepParam.builder().jobId(jobId)
            .logInfo(ProtectionJobStepEnum.PROTECTION_START.getValue())
            .level(JobLogLevelEnum.INFO.getValue())
            .status(JobStatusEnum.RUNNING)
            .build());
        boolean result = protectGroup(dto, sla, jobId);
        if (result) {
            // 成功
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionJobStepEnum.PROTECTION_FINISH.getValue())
                .level(JobLogLevelEnum.INFO.getValue())
                .status(JobStatusEnum.SUCCESS)
                .build());
        } else {
            // 失败
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionJobStepEnum.PROTECTION_FAILED.getValue())
                .level(JobLogLevelEnum.FATAL.getValue())
                .status(JobStatusEnum.FAIL)
                .build());
        }
        return result;
    }

    private boolean protectGroup(ProtectionExecuteDto dto, SlaDto sla, String jobId) {
        boolean result = protectCurrentResource(dto, sla, jobId);
        if (!result) {
            return false;
        }
        List<String> successList = new ArrayList<>();
        List<String> failedList = new ArrayList<>();
        List<ProtectionResourceDto> subResources = dto.getSubResources();
        if (GroupTypeEnum.RULE.getValue().equals(dto.getResource().getGroupType())) {
            JSONObject extParameters = dto.getExtParameters();
            boolean overwrite = extParameters.containsKey("overwrite") && extParameters.getBoolean("overwrite");
            subResources = processSubSources(overwrite, subResources);
        }
        for (ProtectionResourceDto resource : subResources) {
            boolean subResult = protectSubResource(dto, sla, jobId, resource);
            if (subResult) {
                successList.add(resource.getUuid());
            } else {
                failedList.add(resource.getUuid());
            }
        }
        resourceService.batchUpdateStatusById(successList, ProtectStatusEnum.PROTECTED.getProtectStatusNum());
        resourceService.batchUpdateStatusById(failedList, ProtectStatusEnum.UNPROTECTED.getProtectStatusNum());
        return VerifyUtil.isEmpty(failedList);
    }

    private List<ProtectionResourceDto> processSubSources(boolean overwrite,
        List<ProtectionResourceDto> subResources) {
        if (overwrite) {
            List<String> protectedSourceIds = subResources.stream()
                .filter(v -> v.getProtectedObject() != null)
                .map(ProtectionResourceDto::getUuid)
                .collect(Collectors.toList());
            removeProtectedByIds(protectedSourceIds);
            return subResources;
        }
        return subResources.stream()
            .filter(v -> v.getProtectedObject() == null)
            .collect(Collectors.toList());
    }

    private void removeProtectedByIds(List<String> protectedSourceIds) {
        if (VerifyUtil.isEmpty(protectedSourceIds)) {
            return;
        }
        ProtectionBatchOperationReq deleteGroupSubSourceReq = new ProtectionBatchOperationReq();
        deleteGroupSubSourceReq.setIsResourceGroup(false);
        deleteGroupSubSourceReq.setResourceIds(protectedSourceIds);
        protectObjectRestApi.deleteProtectedObjects(deleteGroupSubSourceReq);
    }

    private boolean protectSubResource(ProtectionExecuteDto dto, SlaDto sla, String jobId,
        ProtectionResourceDto resource) {
        List<String> logInfoParams = Arrays.asList(resource.getName(), sla.getName());
        try {
            preProtectCheck(resource, sla, dto.getResource().getGroupType());
            protectResource(dto, sla, resource, false, true);
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionJobStepEnum.PROTECTION_EXECUTING_SUCCESS.getValue())
                .params(logInfoParams)
                .level(JobLogLevelEnum.INFO.getValue())
                .build());
            return true;
        } catch (Exception e) {
            log.error("resource {} execute protection failed,", resource.getUuid(), ExceptionUtil.getErrorMessage(e));
            List<String> detailInfoParams = null;
            String logDetail = null;
            if (e instanceof LegoCheckedException) {
                LegoCheckedException exception = (LegoCheckedException) e;
                logDetail = exception.getErrorCode() + "";
                String[] parameters = exception.getParameters();
                detailInfoParams = VerifyUtil.isEmpty(parameters) ? null : Arrays.asList(parameters);
            }
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionJobStepEnum.PROTECTION_EXECUTING_FAILED.getValue())
                .params(logInfoParams)
                .level(JobLogLevelEnum.ERROR.getValue())
                .detail(logDetail)
                .detailParam(detailInfoParams)
                .build());
            return false;
        }
    }

    private void preProtectCheck(ProtectionResourceDto resource, SlaDto sla, String groupType) {
        checkExistProtected(resource.getUuid(), groupType);
        checkHyperVDiskInfo(resource);
        checkVmwareMatchesSla(resource, sla);
    }

    private void checkExistProtected(String resourceId, String groupType) {
        // 按规则过滤的组不需要校验是否有保护对象，在前面的逻辑中要么会覆盖先移除要么会过滤已经被保护的资源
        if (GroupTypeEnum.RULE.getValue().equals(groupType)) {
            return;
        }
        Long count = protectedObjectMapper.selectCount(
            new LambdaQueryWrapper<ProtectedObjectPo>()
                .eq(ProtectedObjectPo::getResourceId, resourceId));
        if (count > 0) {
            throw new LegoCheckedException(ProtectObjectErrorCode.RESOURCE_ALREADY_PROTECTED,
                "The resource already protected.");
        }
    }

    private void checkHyperVDiskInfo(ProtectionResourceDto resource) {
        if (!ResourceSubTypeEnum.HYPER_V_VM.getType().equals(resource.getSubType())) {
            return;
        }
        Map<String, String> extendInfo = resourceService.getResourceById(false, resource.getUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST))
            .getExtendInfo();
        if (VerifyUtil.isEmpty(extendInfo) || VerifyUtil.isEmpty(extendInfo.get("disks"))) {
            throw new LegoCheckedException(ResourceErrorCodeConstant.VIRTUAL_MACHINE_DISK_INFO_IS_EMPTY,
                "Failed to obtain the cloud server disk information.");
        }
        List<com.alibaba.fastjson.JSONObject> disks = JSON.parseArray(extendInfo.get("disks"),
            com.alibaba.fastjson.JSONObject.class);
        if (VerifyUtil.isEmpty(disks)) {
            throw new LegoCheckedException(ResourceErrorCodeConstant.VIRTUAL_MACHINE_DISK_INFO_IS_EMPTY,
                "Failed to obtain the cloud server disk information.");
        }
        for (com.alibaba.fastjson.JSONObject disk : disks) {
            com.alibaba.fastjson.JSONObject diskExtendInfo = disk.getJSONObject("extendInfo");
            if (VerifyUtil.isEmpty(diskExtendInfo)) {
                continue;
            }
            if (diskExtendInfo.containsKey("IsShared") && diskExtendInfo.getBoolean("IsShared")) {
                throw new LegoCheckedException(ResourceErrorCodeConstant.NOT_SUPPORT_SHARED_DISK,
                    "Fail to protect hyper-v, because not to support shared disk.");
            }
            if (diskExtendInfo.containsKey("IsPhysicalHardDisk") && diskExtendInfo.getBoolean("IsPhysicalHardDisk")) {
                throw new LegoCheckedException(ResourceErrorCodeConstant.NOT_SUPPORT_PHYSICAL_HARD_DISK,
                    "Fail to protect hyper-v, because not to support physical hard disk.");
            }
            if (VHD_SET.equals(diskExtendInfo.get("Format"))) {
                throw new LegoCheckedException(ResourceErrorCodeConstant.NOT_SUPPORT_VHD_SET_DISK,
                    "Fail to protect hyper-v, because not to support vhdx set disk.");
            }
        }
    }

    private void checkVmwareMatchesSla(ProtectionResourceDto resource, SlaDto sla) {
        ResourceSubTypeEnum application = sla.getApplication();
        if (ResourceSubTypeEnum.COMMON.equals(application)
            && VMWARE_TYPES.contains(resource.getSubType())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "The SLA type is inconsistent with the resource type");
        }
    }

    private boolean protectCurrentResource(ProtectionExecuteDto dto, SlaDto sla, String jobId) {
        ResourceGroupDto group = dto.getResource();
        List<String> logInfoParams = Arrays.asList(group.getName(), sla.getName());
        try {
            ProtectionResourceDto resource = new ProtectionResourceDto();
            resource.setUuid(group.getUuid());
            resource.setName(group.getName());
            resource.setPath(group.getPath());
            resource.setType(VerifyUtil.isEmpty(group.getSourceType()) ? "" : group.getSourceType());
            resource.setSubType(VerifyUtil.isEmpty(group.getSourceSubType()) ? "" : group.getSourceSubType());
            resource.setRootUuid("");
            protectResource(dto, sla, resource, true, false);

            // 更新资源组保护状态
            resourceGroupRepository.updateStatusById(group.getUuid(), ProtectStatusEnum.PROTECTED);
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionJobStepEnum.PROTECTION_EXECUTING_SUCCESS.getValue())
                .params(logInfoParams)
                .level(JobLogLevelEnum.INFO.getValue())
                .build());
            return true;
        } catch (Exception e) {
            log.error("resource {} execute protection failed,", group.getUuid(), ExceptionUtil.getErrorMessage(e));
            String logDetail = null;
            List<String> detailInfoParams = null;
            if (e instanceof LegoCheckedException) {
                LegoCheckedException exception = (LegoCheckedException) e;
                logDetail = exception.getErrorCode() + "";
                String[] parameters = exception.getParameters();
                detailInfoParams = VerifyUtil.isEmpty(parameters) ? null : Arrays.asList(parameters);
            }
            // 更新资源组保护状态
            resourceGroupRepository.updateStatusById(group.getUuid(), ProtectStatusEnum.UNPROTECTED);
            recordJobStep(JobStepParam.builder().jobId(jobId)
                .logInfo(ProtectionJobStepEnum.PROTECTION_EXECUTING_FAILED.getValue())
                .params(logInfoParams)
                .level(JobLogLevelEnum.ERROR.getValue())
                .detail(logDetail)
                .detailParam(detailInfoParams)
                .build());
            return false;
        }
    }

    private void protectResource(ProtectionExecuteDto dto, SlaDto sla, ProtectionResourceDto resource, boolean isGroup,
        boolean isGroupSubSource) {
        ProtectedObjectPo protectedObject = new ProtectedObjectPo();
        ResourceGroupDto group = dto.getResource();

        // 保持和资源组一样的保护，可能是创建保护后禁用状态
        int status = VerifyUtil.isEmpty(group.getProtectedObjectDto())
            ? ProtectStatusEnum.PROTECTED.getProtectStatusNum() : group.getProtectedObjectDto().getStatus();
        protectedObject.setUuid(resource.getUuid());
        protectedObject.setResourceId(resource.getUuid());
        protectedObject.setResourceGroupId(group.getUuid());
        protectedObject.setSlaId(sla.getUuid());
        protectedObject.setSlaName(sla.getName());
        protectedObject.setExtParameters(dto.getExtParameters().toString());
        protectedObject.setName(resource.getName());
        protectedObject.setPath(resource.getPath());
        protectedObject.setType(resource.getType());
        protectedObject.setSubType(resource.getSubType());
        protectedObject.setEnvId(resource.getRootUuid());
        protectedObject.setChainId(UUID.randomUUID().toString());
        protectedObject.setStatus(status);
        List<PolicyDto> policyList = sla.getPolicyList();
        List<ProtectedTaskPo> taskList = new ArrayList<>();
        for (PolicyDto policy : policyList) {
            if (filterSlaPolicy(policy, isGroup, isGroupSubSource)) {
                String topic = isGroup ? GROUP_BACKUP_TOPIC : policy.getType().getType();
                String scheduleTopic = isGroup ? topic : "schedule." + topic;
                taskList.add(buildProtectionTask(resource.getUuid(), policy, scheduleTopic, buildScheduleParams(
                    topic, resource.getUuid(), sla.getUuid(), protectedObject.getChainId(), policy
                )));
            }
        }
        protectedObjectMapper.insertProtectedObject(protectedObject);
        if (!VerifyUtil.isEmpty(taskList)) {
            projectedTaskService.saveBatch(taskList);
        }
    }

    private ProtectedTaskPo buildProtectionTask(String resourceId, PolicyDto policy, String topic, JSONObject params) {
        ProtectedTaskPo task = new ProtectedTaskPo();
        String taskId = generateUUID5(NAMESPACE_OID, resourceId + policy.getUuid()).toString();
        ScheduleDto schedule = constructScheduleStartTime(policy.getSchedule());
        startScheduler(topic, schedule, params, taskId);

        task.setUuid(taskId);
        task.setPolicyId(policy.getUuid());
        task.setProtectedObjectId(resourceId);
        task.setScheduleId(taskId);
        return task;
    }

    private ScheduleDto constructScheduleStartTime(ScheduleDto dto) {
        ScheduleDto schedule = new ScheduleDto();
        BeanUtils.copyProperties(dto, schedule);
        if (Trigger.CUSTOMIZE_INTERVAL.equals(dto.getTrigger()) && TriggerAction.YEAR.equals(dto.getTriggerAction())) {
            schedule.setInterval(1);
            schedule.setIntervalUnit("Y");
        }
        if (Trigger.CUSTOMIZE_INTERVAL.equals(dto.getTrigger()) && TriggerAction.WEEK.equals(dto.getTriggerAction())) {
            schedule.setDaysOfWeek(WeekDayEnum.convertName2Index(dto.getDaysOfWeek()));
        }
        return schedule;
    }

    private void startScheduler(String topic, ScheduleDto dto, JSONObject params, String taskId) {
        Schedule schedule = new Schedule();
        schedule.setScheduleName(taskId);
        schedule.setAction(topic);
        schedule.setParams(params.toString());
        if (Trigger.CUSTOMIZE_INTERVAL.equals(dto.getTrigger()) && !TriggerAction.YEAR.equals(dto.getTriggerAction())) {
            if (!VerifyUtil.isEmpty(dto.getDaysOfWeek())) {
                schedule.setDatOfWeek(String.join(",", dto.getDaysOfWeek()));
            }
            schedule.setDayOfMonth(dto.getDaysOfMonth());
            schedule.setDailyStartTime(dto.getWindowStart());
            schedule.setDailyEndTime(dto.getWindowEnd());
            schedule.setScheduleType(dto.getTriggerAction().getAction());
        } else {
            schedule.setInterval(dto.getInterval() + dto.getIntervalUnit());
            schedule.setScheduleType(ScheduleType.INTERVAL.getType());
            schedule.setStartDate(dto.getStartTime());
        }
        try {
            scheduleService.startScheduler(schedule);
        } catch (SchedulerException e) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Create interval schedule task failed", e);
        }
    }

    private JSONObject buildScheduleParams(String topic, String resourceId, String slaId,
        String chainId, PolicyDto policy) {
        JSONObject params = new JSONObject();
        params.set("resource_id", resourceId);
        if (SCAN_VM_UNDER_COMPUTE_RES.equals(topic)) {
            return params;
        }
        params.set("sla_id", slaId);
        params.set("chain_id", chainId);
        params.set("policy", policy);
        params.set("execute_type", AUTOMATIC);
        if (PolicyType.ARCHIVING.getType().equals(topic) || PolicyType.REPLICATION.getType().equals(topic)) {
            return params;
        }
        if (GROUP_BACKUP_TOPIC.equals(topic) || PolicyType.BACKUP.getType().equals(topic)) {
            JsonNode parameters = policy.getExtParameters();
            boolean autoRetry = false;
            if (!VerifyUtil.isEmpty(parameters)) {
                autoRetry = parameters.get("auto_retry").booleanValue();
                params.set("auto_retry", autoRetry);
            }
            if (autoRetry) {
                params.set("auto_retry_times", parameters.get("auto_retry_times"));
                params.set("auto_retry_wait_minutes", parameters.get("auto_retry_wait_minutes"));
            }
            return params;
        }
        throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
            String.format("create schedule topic[%s] is not supported", topic));
    }

    private static UUID generateUUID5(UUID namespace, String name) {
        try {
            // 使用 SHA-1 来生成 UUID5
            MessageDigest sha1 = MessageDigest.getInstance("SHA-1");
            sha1.update(namespace.toString().getBytes(StandardCharsets.UTF_8));
            sha1.update(name.getBytes(StandardCharsets.UTF_8));
            byte[] hash = sha1.digest();

            // 设置 UUID v5 的版本（SHA-1 哈希的第 6 字节，设置为 5）;清除高四位 设置为版本 5
            hash[6] &= 0x0f;
            hash[6] |= 0x50;

            // 设置 UUID 的变体（第 8 字节，设置为 10; 清除高两位 设置变体为 10
            hash[8] &= 0x3f;
            hash[8] |= (byte) 0x80;

            // 将哈希值转换为 UUID
            long msb = 0L;
            long lsb = 0L;

            for (int i = 0; i < 8; i++) {
                msb = (msb << 8) | (hash[i] & 0xff);
            }
            for (int i = 8; i < 16; i++) {
                lsb = (lsb << 8) | (hash[i] & 0xff);
            }

            return new UUID(msb, lsb);
        } catch (Exception e) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Error generating UUID", e);
        }
    }

    private boolean filterSlaPolicy(PolicyDto policy, boolean isGroup, boolean isGroupSubSource) {
        if (isGroup) {
            return policy.getType() == PolicyType.BACKUP;
        }
        if (isGroupSubSource) {
            return policy.getType() != PolicyType.BACKUP;
        }
        if (PolicyType.REPLICATION.equals(policy.getType()) || PolicyType.ARCHIVING.equals(policy.getType())) {
            ScheduleDto schedule = policy.getSchedule();
            return !VerifyUtil.isEmpty(schedule) && schedule.getTrigger() == Trigger.INTERVAL;
        }
        return policy.getType() == PolicyType.BACKUP;
    }

    private CreateJobRequest buildJobReq(ResourceGroupDto groupDto, TokenBo.UserBo user, SlaDto sla,
        JobTypeEnum jobType) {
        String jobId = UUID.randomUUID().toString();
        CreateJobRequest jobReq = new CreateJobRequest();
        jobReq.setRequestId(jobId);
        JSONObject extendParams = new JSONObject();
        extendParams.put("sla_name", sla.getName());
        extendParams.put("sla_id", sla.getUuid());
        jobReq.setSourceId(groupDto.getUuid());
        jobReq.setSourceName(groupDto.getName());
        jobReq.setSourceType(groupDto.getSourceType());
        jobReq.setSourceSubType(groupDto.getSourceSubType());
        jobReq.setSourceLocation("");
        jobReq.setType(jobType.getValue());
        jobReq.setUserId(user.getId());
        jobReq.setDomainIdList(resourceSetApi.getRelatedDomainIdList(groupDto.getUuid()));
        jobReq.setExtendField(extendParams);
        return jobReq;
    }

    private void recordJobStep(JobStepParam param) {
        UpdateJobRequest updateJobReq = new UpdateJobRequest();
        JobStatusEnum status = param.getStatus();
        List<String> params = param.getParams();
        if (status != null) {
            updateJobReq.setStatus(status);
            updateJobReq.setProgress(JobStatusEnum.RUNNING.equals(status) ? 10 : 100);
        }
        JobLogBo jobLog = new JobLogBo();
        jobLog.setJobId(param.getJobId());
        jobLog.setStartTime(System.currentTimeMillis());
        jobLog.setLogInfo(param.getLogInfo());
        jobLog.setLogInfoParam(params == null ? new ArrayList<>() : params);
        jobLog.setLevel(param.getLevel());
        jobLog.setLogDetail(param.getDetail());
        jobLog.setLogDetailParam(param.getDetailParam());
        updateJobReq.setJobLogs(Collections.singletonList(jobLog));
        jobService.updateJob(param.getJobId(), updateJobReq);
    }

    private void fillParam(ProtectionExecuteDto dto, SlaDto sla) {
        List<PolicyDto> policyList = sla.getPolicyList();
        if (VerifyUtil.isEmpty(policyList)) {
            dto.getExtParameters().put("worm_switch", false);
            return;
        }
        for (PolicyDto policy : policyList) {
            Integer wormValidityType = policy.getWormValidityType();
            PolicyType type = policy.getType();
            if (PolicyType.BACKUP.equals(type)
                && WormValidityTypeEnum.WORM_NOT_OPEN.getType().equals(wormValidityType)) {
                dto.getExtParameters().put("worm_switch", true);
                return;
            }
        }
    }
}
