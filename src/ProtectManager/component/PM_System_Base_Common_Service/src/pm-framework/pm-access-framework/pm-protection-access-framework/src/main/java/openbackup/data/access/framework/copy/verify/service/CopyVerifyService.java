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
package openbackup.data.access.framework.copy.verify.service;

import static openbackup.data.protection.access.provider.sdk.constants.RestoreTaskExtendInfoConstant.COPY_VERIFY_RANGE_END;
import static openbackup.data.protection.access.provider.sdk.constants.RestoreTaskExtendInfoConstant.COPY_VERIFY_RANGE_START;

import com.huawei.oceanprotect.job.dto.JobProgressRange;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.protection.service.job.InternalApiHub;
import openbackup.data.access.framework.restore.service.RestoreTaskHelper;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.lock.LockRequest;
import openbackup.system.base.service.AvailableAgentManagementDomainService;

import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.stereotype.Service;

import java.net.URI;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

/**
 * 副本校验服务类
 *
 **/
@Slf4j
@Service
public class CopyVerifyService {
    private static final String ENFORCE_STOP = "enforce-stop";

    private final InternalApiHub internalApiHub;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final KafkaTemplate<String, String> template;

    private final AvailableAgentManagementDomainService domainService;

    private final ResourceSetApi resourceSetApi;

    /**
     * 副本校验服务构造函数
     *
     * @param internalApiHub 内部api集合
     * @param dmeUnifiedRestApi dme统一rest服务
     * @param template kafka消息发送
     * @param domainService 代理
     * @param resourceSetApi 资源集sdk
     */
    public CopyVerifyService(InternalApiHub internalApiHub, DmeUnifiedRestApi dmeUnifiedRestApi,
        KafkaTemplate<String, String> template, AvailableAgentManagementDomainService domainService,
        ResourceSetApi resourceSetApi) {
        this.template = template;
        this.internalApiHub = internalApiHub;
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
        this.domainService = domainService;
        this.resourceSetApi = resourceSetApi;
    }

    /**
     * 创建副本校验任务
     *
     * @param context 副本校验任务上下文
     * @return 任务id
     */
    String createJob(CopyVerifyManagerContext context) {
        CreateJobRequest createJobRequest = CopyVerifyHelper.buildCreateJobReq(context);
        createJobRequest.setDomainIdList(resourceSetApi.getRelatedDomainIdList(createJobRequest.getSourceId()));
        log.info("Copy Verify create job, requestId={}", context.getRequestId());
        return internalApiHub.getJobService().createJob(createJobRequest);
    }

    /**
     * 根据副本id查询副本详情
     *
     * @param copyId 副本id
     * @return 副本信息
     */
    Copy getCopyDetail(String copyId) {
        return internalApiHub.getCopyRestApi().queryCopyByID(copyId);
    }

    /**
     * 开始副本校验任务
     *
     * @param task 副本校验任务信息
     */
    void start(CopyVerifyTask task) {
        URI uri = domainService.getUrlByAgents(AgentApiUtil.getAgentIds(task.getAgents()));
        log.info("CopyVerify uri:{}, task id:{}", JSONObject.stringify(uri), task.getRequestId());

        log.info("start to send CopyVerify task to dme, requestId is {}", task.getRequestId());
        dmeUnifiedRestApi.checkCopy(uri, task);
        log.info("Send CopyVerify task to dme successful! requestId is {}", task.getRequestId());
    }

    boolean lockResource(String requestId, String taskId, String copyId) {
        final LockRequest lockRequest = CopyVerifyHelper.buildLockRequest(requestId, taskId, copyId);
        return internalApiHub.getLockService().lock(lockRequest);
    }

    /**
     * 从任务中获取副本校验任务信息
     *
     * @param jobId 任务id
     * @return 副本校验任务信息
     */
    public CopyVerifyTask getCopyCheckTaskFromJob(String jobId) {
        final JobBo jobBo = internalApiHub.getJobService().queryJob(jobId);
        return CopyVerifyHelper.parseFromJobMessage(jobBo.getMessage());
    }

    /**
     * 解锁资源
     *
     * @param requestId 请求id
     * @param taskId 任务id
     * @param isNeedLog 是否需要记录解锁步骤日志
     */
    public void unlockResource(String requestId, String taskId, boolean isNeedLog) {
        internalApiHub.getLockService().unlock(requestId, taskId, isNeedLog);
    }

    /**
     * 执行恢复任务
     *
     * @param requestId 请求id（任务id）
     */
    public void executeRestoreTask(String requestId) {
        final JobBo jobBo = internalApiHub.getJobService().queryJob(requestId);
        final RestoreTask restoreTask = RestoreTaskHelper.parseFromJobMessage(jobBo.getMessage());
        template.send(TopicConstants.RESTORE_EXECUTE_V2, JSONObject.fromObject(restoreTask).toString());
    }

    void modifyJobStatusRunning(String jobId) {
        internalApiHub.getJobService()
            .updateJob(jobId, RestoreTaskHelper.buildJobRequestWithStatus(ProviderJobStatusEnum.RUNNING));
    }

    void modifyJobCanNotForceStop(String jobId) {
        internalApiHub.getJobService()
            .updateJob(jobId, RestoreTaskHelper.buildJobRequestWithData(new JSONObject().set(ENFORCE_STOP, false)));
    }

    void modifyJobProgressRange(String jobId) {
        internalApiHub.getJobService()
            .modifyProgressRange(jobId, new JobProgressRange(COPY_VERIFY_RANGE_START, COPY_VERIFY_RANGE_END));
    }

    void completeJob(String jobId, ProviderJobStatusEnum status) {
        internalApiHub.getJobService().updateJob(jobId, RestoreTaskHelper.buildJobRequestWithStatus(status));
    }

    void modifyCopyStatus(String copyId, CopyStatus status) {
        log.info("Copy Verify task update copy status, copyId: {}, status: {}", copyId, status.name());
        internalApiHub.getCopyRestApi().updateCopyStatus(copyId, RestoreTaskHelper.buildUpdateCopyStatusReq(status));
    }

    void modifyCopyVerifyStatus(String copyId, CopyVerifyStatusEnum verifyStatus) {
        internalApiHub.getCopyRestApi()
            .updateProperties(copyId, CopyVerifyHelper.buildUpdateRequest(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS,
                verifyStatus.getVerifyStatus()));
    }

    void modifyLastVerifyTime(String copyId) {
        final String nowDatetimeStr = LocalDateTime.now().format(DateTimeFormatter.ISO_LOCAL_DATE_TIME);
        internalApiHub.getCopyRestApi()
            .updateProperties(copyId,
                CopyVerifyHelper.buildUpdateRequest(CopyPropertiesKeyConstant.KEY_LAST_VERIFY_TIME, nowDatetimeStr));
    }

    void postProcess(CopyVerifyTask task, boolean isDamaged) {
        Copy copy = internalApiHub.getCopyRestApi().queryCopyByID(task.getCopyId());
        // 查询最新的备份副本
        Copy latestBackupCopy = internalApiHub.getCopyRestApi()
                .queryLatestBackupCopy(copy.getResourceId(), null, null);
        if (latestBackupCopy != null) {
            log.warn("Copy Verify Fail. requestId={}, latestCopy Uuid={}",
                    task.getTaskId(), latestBackupCopy.getUuid());
            if (latestBackupCopy.getUuid().equals(copy.getUuid()) && isDamaged) {
                log.warn("Copy Verify Fail and setNextBackupToFull, requestId={}, resourceId={}", task.getTaskId(),
                        copy.getResourceId());
                NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(
                        copy.getResourceId(), NextBackupChangeCauseEnum.VERIFY_FAILED_TO_FULL_LABEL);
                internalApiHub.getResourceService().modifyNextBackup(nextBackupModifyReq, false);
            }
        }
    }
}
