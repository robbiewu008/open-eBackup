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
package openbackup.data.access.framework.restore.service;

import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.restore.constant.RestoreConstant;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.dto.RestoreTaskContext;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.constants.RestoreTaskExtendInfoConstant;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JSONObjectCovert;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateParam;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.lock.LockRequest;
import openbackup.system.base.sdk.lock.LockResource;
import openbackup.system.base.sdk.lock.LockTypeEnum;
import openbackup.system.base.util.SpringBeanUtils;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.util.Assert;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * 恢复任务辅助类，提供一些静态辅助函数
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/28
 **/
@Slf4j
public abstract class RestoreTaskHelper {
    private RestoreTaskHelper() {
    }

    /**
     * 根据副本状态构建副本更新请求
     *
     * @param status 副本需要修改到的状态
     * @return 副本状态更新请求
     */
    public static CopyStatusUpdateParam buildUpdateCopyStatusReq(CopyStatus status) {
        CopyStatusUpdateParam updateRequest = new CopyStatusUpdateParam();
        updateRequest.setStatus(status);
        return updateRequest;
    }

    private static JobTypeEnum covertToJobCenterType(RestoreTypeEnum restoreType) {
        switch (restoreType) {
            case CR:
            case FLR:
                return JobTypeEnum.RESTORE;
            case IR:
                return JobTypeEnum.INSTANT_RESTORE;
            default:
                throw new IllegalArgumentException("Unsupported value: " + restoreType);
        }
    }

    /**
     * 从副本信息中获取并构造TaskResource
     * <p>
     * 适用于原位置恢复时，targetObject参数为空的场景
     * </p>
     *
     * @param copy 副本信息
     * @return 任务资源对象 {@code TaskResource}
     */
    public static TaskResource buildTaskResourceByCopy(final Copy copy) {
        final String resourceProperties = copy.getResourceProperties();
        final JSONObject targetJsonObject = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(
                JSONObject.fromObject(resourceProperties));
        final ProtectedResource protectedResource = targetJsonObject.toBean(ProtectedResource.class);
        TaskResource taskResource = new TaskResource();
        BeanUtils.copyProperties(protectedResource, taskResource);
        return taskResource;
    }

    /**
     * 构造锁定资源请求
     *
     * @param requestId 请求id
     * @param copyId    副本id
     * @param resources 需要锁定的资源
     * @return 资源锁定请求
     */
    public static LockRequest buildLockRequest(String requestId, String copyId, List<LockResourceBo> resources) {
        LockRequest lockRequest = new LockRequest();
        lockRequest.setRequestId(requestId);
        lockRequest.setLockId(requestId);
        lockRequest.setPriority(RestoreConstant.RESTORE_LOCK_PRIORITY);
        List<LockResource> lockResourceList = new ArrayList<>();
        lockResourceList.add(new LockResource(copyId, LockTypeEnum.READ));
        // 给关联副本上锁
        CopyManagerService copyManagerService = SpringBeanUtils.getBean(CopyManagerService.class);
        List<String> copies = copyManagerService.getAssociatedCopies(copyId);
        copies.stream()
            .filter(otherCopyId -> !otherCopyId.equals(copyId))
            .forEach(otherCopyId -> lockResourceList.add(new LockResource(otherCopyId, LockTypeEnum.READ)));
        resources.forEach(item -> lockResourceList.add(
                new LockResource(item.getId(), LockTypeEnum.getByType(item.getLockType().getType()))));
        lockRequest.setResources(lockResourceList);
        return lockRequest;
    }

    /**
     * 从任务消息中解析恢复任务对象
     *
     * @param message 任务消息，json格式字符串
     * @return 恢复任务对象
     */
    public static RestoreTask parseFromJobMessage(String message) {
        Assert.hasLength(message, "Restore job message is empty");
        final JobMessage jobMessage = JSONObject.toBean(message, JobMessage.class);
        final JSONObject payload = jobMessage.getPayload();
        Assert.notEmpty(payload, "Restore job payload is empty");
        return payload.toBean(RestoreTask.class);
    }

    /**
     * 构造任务更新请求
     *
     * @param jobData 任务高级参数
     * @return 任务更新请求
     */
    public static UpdateJobRequest buildJobRequestWithData(JSONObject jobData) {
        UpdateJobRequest updateRequest = new UpdateJobRequest();
        updateRequest.setData(jobData);
        return updateRequest;
    }

    /**
     * 构造更新任务状态请求
     *
     * @param status 需要更新到的任务状态
     * @return 更新任务状态请求
     */
    public static UpdateJobRequest buildJobRequestWithStatus(ProviderJobStatusEnum status) {
        UpdateJobRequest updateRequest = new UpdateJobRequest();
        updateRequest.setStatus(JobStatusEnum.get(status.name()));
        return updateRequest;
    }

    /**
     * 根据恢复目标选择创建Job是目标位置的信息
     *
     * @param location       恢复目标枚举
     * @param sourceLocation 源资源位置
     * @param targetLocation 目标资源位置
     * @return 选则的目标位置信息
     */
    private static String choseTargetLocation(RestoreLocationEnum location, String sourceLocation,
        String targetLocation) {
        switch (location) {
            case NEW:
                return targetLocation;
            case ORIGINAL:
                return sourceLocation;
            case NATIVE:
                return RestoreConstant.TARGET_LOCATION_NATIVE;
            default:
                throw new IllegalStateException("Unsupported value: " + location);
        }
    }

    private static JobMessage buildRestoreStartMessage(RestoreTask restoreTask) {
        JobMessage message = new JobMessage();
        message.setInContext(false);
        // 加密恢复任务信息
        restoreTask.encryptRestoreTask();
        message.setPayload(JSONObject.fromObject(restoreTask));
        // 设置不同的topic来触发后续不同的任务流程
        if (isEnableCopyVerify(restoreTask.getAdvanceParams())) {
            message.setTopic(TopicConstants.COPY_VERIFY_EXECUTE);
        } else {
            message.setTopic(TopicConstants.RESTORE_EXECUTE_V2);
        }
        return message;
    }

    private static JSONObject buildRestoreExtendField(RestoreTask restoreTask, String copyPropertiesStr,
        String copyOriginalStatus, CreateRestoreTaskRequest createRestoreTaskRequest) {
        JSONObject extendField = new JSONObject();
        if (restoreTask.getRestoreType().equals(RestoreTypeEnum.FLR.getType())) {
            extendField.set(JobExtendInfoKeys.RESTORE_TYPE, RestoreTypeEnum.FLR.getType());
        }
        JSONObject copyProperties = JSONObject.fromObject(copyPropertiesStr);
        if (StringUtils.isNotEmpty(copyProperties.getString(CopyPropertiesKeyConstant.KEY_BACKUP_REPOSITORY_ID))) {
            extendField.set(JobExtendInfoKeys.STORAGE_ID,
                    copyProperties.getString(CopyPropertiesKeyConstant.KEY_BACKUP_REPOSITORY_ID));
        }
        JSONObject extendInfoClone = JSONObject.fromObject(createRestoreTaskRequest.getExtendInfo());
        if (createRestoreTaskRequest.getScripts() != null) {
            extendInfoClone.putAll(
                JsonUtil.read(createRestoreTaskRequest.getScripts(), new TypeReference<Map<String, String>>() {}));
        }
        if (createRestoreTaskRequest.getAgents() != null) {
            extendInfoClone.put(JobExtendInfoKeys.AGENTS, createRestoreTaskRequest.getAgents());
        }

        extendField.set(JobExtendInfoKeys.JOB_CONFIG, extendInfoClone);
        extendField.set(JobExtendInfoKeys.RESTORE_TARGET_LOCATION, restoreTask.getTargetLocation().getLocation());
        extendField.set(JobExtendInfoKeys.RESTORE_COPY_ORIGINAL_STATUS, copyOriginalStatus);
        return extendField;
    }

    /**
     * 构造恢复任务创建请求
     *
     * @param context 恢复任务上下文
     * @return 恢复任务创建请求
     */
    public static CreateJobRequest buildRestoreJobRequest(RestoreTaskContext context) {
        final CreateRestoreTaskRequest request = context.getTaskRequest();
        final RestoreTask restoreTask = context.getRestoreTask();
        CreateJobRequest jobRequest = new CreateJobRequest();
        jobRequest.setCopyId(request.getCopyId());
        jobRequest.setIsSystem(Boolean.FALSE);
        jobRequest.setRequestId(context.getRequestId());
        jobRequest.setType(covertToJobCenterType(request.getRestoreType()).name());
        jobRequest.setTargetName(restoreTask.getTargetObject().getName());
        jobRequest.setExerciseJobId(context.getTaskRequest().getExerciseJobId());
        jobRequest.setExerciseId(context.getTaskRequest().getExerciseId());
        // 设置恢复任务源对象信息
        final Copy copy = context.getCopy();
        jobRequest.setSourceId(copy.getResourceId());
        jobRequest.setSourceName(copy.getResourceName());
        jobRequest.setSourceLocation(copy.getLocation());
        jobRequest.setSourceType(copy.getResourceType());
        jobRequest.setSourceSubType(copy.getResourceSubType());
        jobRequest.setSourceLocation(copy.getResourceLocation());
        jobRequest.setCopyTime(Long.valueOf(copy.getTimestamp()));
        jobRequest.setUserId(copy.getUserId());

        setRestoreTargetLocation(request, restoreTask, jobRequest, copy);
        // 根据副本生成类型判断当前资源的恢复任务是否支持停止
        jobRequest.setEnableStop(Boolean.FALSE);
        // 设置恢复任务目标对象消息
        // 设置任务消息数据，JobCenter处理任务限流之后，会按照消息中的topic,
        // 把消息中的数据作为kafka消息体发出来
        jobRequest.setMessage(buildRestoreStartMessage(restoreTask));
        jobRequest.setExtendField(
            buildRestoreExtendField(restoreTask, copy.getProperties(), copy.getStatus(), request));
        return jobRequest;
    }

    private static void setRestoreTargetLocation(CreateRestoreTaskRequest request, RestoreTask restoreTask,
        CreateJobRequest jobRequest, Copy copy) {
        if (!StringUtils.isEmpty(restoreTask.getAdvanceParams().get(RestoreConstant.RESTORE_LOCATION))) {
            jobRequest.setTargetLocation(restoreTask.getAdvanceParams().get(RestoreConstant.RESTORE_LOCATION));
        } else if (!StringUtils.isEmpty(restoreTask.getTargetObject().getTargetLocation())) {
            jobRequest.setTargetLocation(restoreTask.getTargetObject().getTargetLocation());
        } else {
            jobRequest.setTargetLocation(choseTargetLocation(request.getTargetLocation(), copy.getResourceLocation(),
                restoreTask.getTargetObject().getName()));
        }
    }

    /**
     * 恢复任务是否开启副本校验
     *
     * @param extendParams 恢复任务扩展参数
     * @return true-开启；false-未开启
     */
    public static boolean isEnableCopyVerify(Map<String, String> extendParams) {
        if (extendParams == null) {
            return false;
        }
        final String copyVerify = extendParams.getOrDefault(RestoreTaskExtendInfoConstant.ENABLE_COPY_VERIFY, "false");
        return "true".equals(copyVerify);
    }

    /**
     * 恢复任务对象转换为副本校验对象
     *
     * @param restoreTask 恢复任务对象
     * @return 副本校验任务对象
     */
    public static CopyVerifyTask covertToCopyCheckTask(RestoreTask restoreTask) {
        CopyVerifyTask copyVerifyTask = new CopyVerifyTask();
        BeanUtils.copyProperties(restoreTask, copyVerifyTask);
        if (isEnableCopyVerify(restoreTask.getAdvanceParams())) {
            String verifyTaskId = UUID.randomUUID().toString();
            log.info("Copy Verify subTask execute, generate sub taskId, requestId={}, taskId={}",
                    copyVerifyTask.getRequestId(), verifyTaskId);
            copyVerifyTask.setTaskId(verifyTaskId);
        }
        return copyVerifyTask;
    }
}
