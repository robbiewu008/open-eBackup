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
package openbackup.data.access.framework.copy.mng.handler.v2;

import openbackup.data.access.framework.copy.index.service.impl.UnifiedCopyIndexService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.constants.JobStatusLabelConstant;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.RetryTemplateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobStopParam;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.retry.RecoveryCallback;
import org.springframework.retry.RetryCallback;
import org.springframework.retry.support.RetryTemplate;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * 统一副本删除任务完成处理器
 *
 */
@Slf4j
@Component
public class UnifiedCopyDeleteCompleteHandler extends UnifiedTaskCompleteHandler {
    private static final String COPY_DAMAGED = "copy_damaged";

    private final NotifyManager notifyManager;

    private final CopyRestApi copyRestApi;

    private final JobService jobService;

    private final UnifiedCopyIndexService unifiedCopyIndexService;

    private final JobCenterRestApi jobCenterRestApi;

    private CopyManagerService copyManagerService;

    private UserQuotaManager userQuotaManager;

    private ProviderManager providerManager;

    public UnifiedCopyDeleteCompleteHandler(NotifyManager notifyManager, CopyRestApi copyRestApi, JobService jobService,
        UnifiedCopyIndexService unifiedCopyIndexService, JobCenterRestApi jobCenterRestApi) {
        this.notifyManager = notifyManager;
        this.copyRestApi = copyRestApi;
        this.jobService = jobService;
        this.unifiedCopyIndexService = unifiedCopyIndexService;
        this.jobCenterRestApi = jobCenterRestApi;
    }

    @Autowired
    public void setCopyManagerService(CopyManagerService copyManagerService, ProviderManager providerManager) {
        this.copyManagerService = copyManagerService;
        this.providerManager = providerManager;
    }

    @Autowired
    public void setUserQuotaManager(UserQuotaManager userQuotaManager) {
        this.userQuotaManager = userQuotaManager;
    }

    /**
     * 处理卸载任务失败完成逻辑
     *
     * @param taskMessage taskMessage
     */
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskMessage) {
        processTaskComplete(taskMessage);
    }

    /**
     * 处理卸载任务失败完成
     *
     * @param taskMessage taskMessage
     */
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskMessage) {
        // 失败的逻辑在副本模块已做处理，本次仅先做接口适配
        processTaskComplete(taskMessage);
    }

    private void processTaskComplete(TaskCompleteMessageBo taskMessage) {
        updateTaskCanStop(taskMessage);
        JSONObject extendsInfo = JSONObject.fromObject(taskMessage.getExtendsInfo());

        Copy copy = null;
        String jobId = taskMessage.getJobRequestId();
        try {
            copy = copyManagerService.queryCopyFromJobId(jobId);
        } catch (Exception e) {
            String message = String.format("query copy occurs error. job id: %s. message is %s", jobId, e.getMessage());
            log.error(message, ExceptionUtil.getErrorMessage(e));
        }

        int status = taskMessage.getJobStatus();
        // backup_damaged 中的false表示副本未删除成功，可用true表示副本不可用
        boolean isCopyDamaged = DmeJobStatusEnum.SUCCESS.equals(DmeJobStatusEnum.fromStatus(status))
                || ("true".equals(extendsInfo.get("backup_damaged")));
        Map<String, String> context = taskMessage.getContext();
        context.put(COPY_DAMAGED, String.valueOf(isCopyDamaged));
        // 针对应用处理删除成功的后置操作
        if (!VerifyUtil.isEmpty(copy)) {
            copyDeletePostProcess(copy, taskMessage);
        }
        // 向dee发送删除副本的索引通知消息
        String requestId = taskMessage.getJobRequestId();
        try {
            deleteCopyIndex(taskMessage, requestId);
        } catch (Throwable throwable) {
            // 捕获删除索引的所有异常，逻辑继续向下走，任务需要结束
            log.error("Delete copy index failed, requestId: {}.", requestId, throwable);
        }
        // 通知复制微服务该副本已被删除
        try {
            log.debug("start to notify copy delete. job id: {}", jobId);
            copyManagerService.notifyWhenCopyDeleted(jobId, copy);
        } catch (Exception e) {
            String message = String.format("notify copy deleted failed. job id: %s, error message is: %s", jobId,
                e.getMessage());
            log.error(message, e);
        }
        // 副本删除成功，减少用户已使用配额
        userQuotaManager.decreaseUsedQuota(jobId, copy);

        // 发送删除任务完成消息
        sendCopyDeleteCompletedMessage(requestId);

        // 查询与全量副本相关联的所有日志副本，删除全量时，日志副本需要一并删除
        List<String> copyList = queryRelativeCopies(extendsInfo);
        log.debug("Query relative copy list, requestId: {}", requestId);
        // 如果没有与全量相关联的副本则退出
        if (copyList.isEmpty()) {
            return;
        }
        // 删除与全量副本相关联日志副本
        deleteRelativeCopies(copyList, requestId);
    }

    private void copyDeletePostProcess(Copy copy, TaskCompleteMessageBo taskMessage) {
        CopyDeleteInterceptor copyDeleteInterceptor = providerManager.findProvider(CopyDeleteInterceptor.class,
            copy.getResourceSubType(), null);
        if (copyDeleteInterceptor != null) {
            log.debug("Copy delete post process for {} is provided! requestId: {}, copy id: {}",
                copy.getResourceSubType(), taskMessage.getJobRequestId(), copy.getUuid());
            copyDeleteInterceptor.finalize(copy, taskMessage);
        }
    }


    private void deleteCopyIndex(TaskCompleteMessageBo taskMessage, String requestId) throws Throwable {
        String copyId = taskMessage.getContext().get(ContextConstants.COPY_ID);
        RetryCallback<Object, Throwable> retryCallback = (context) -> {
            unifiedCopyIndexService.deleteCopyIndex(requestId, copyId);
            return true;
        };
        RecoveryCallback<Object> recoveryCallback = (context) -> null;
        // 失败需要重试，设置重试策略，重试次数3次, 每次间隔时间10秒，捕获FeignException
        RetryTemplate retryTemplate = RetryTemplateUtil.fixedBackOffRetryTemplate(LegoNumberConstant.THREE,
            LegoNumberConstant.TEN * 1000L, Collections.singletonMap(Exception.class, true));
        retryTemplate.execute(retryCallback, recoveryCallback);
    }

    private void sendCopyDeleteCompletedMessage(String requestId) {
        JSONObject copyDeleteReq = new JSONObject();
        copyDeleteReq.put(ContextConstants.REQUEST_ID, requestId);
        notifyManager.send(TopicConstants.COPY_DELETE_JOB_MONITOR_FINISHED, copyDeleteReq.toString());
    }

    private List<String> queryRelativeCopies(JSONObject extendsInfo) {
        if (extendsInfo == null) {
            return Collections.emptyList();
        }
        JSONArray deletableCopies = JSONArray.fromObject(extendsInfo.get("relative_copies"));
        if (deletableCopies == null) {
            return Collections.emptyList();
        }
        return JSONArray.toCollection(deletableCopies, String.class);
    }

    private void recordRelativeCopiesDeleteLog(String jobId, List<String> copyList) {
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        List<String> logDetailParam = new ArrayList<>();
        logDetailParam.add(copyList.toString());
        jobLogBo.setLogInfoParam(logDetailParam);
        jobLogBo.setLogInfo("delete_relatived_copies_label");
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setJobLogs(Collections.singletonList(jobLogBo));
        jobService.updateJob(jobId, updateJobRequest);
    }

    private void deleteRelativeCopies(List<String> deletableCopyList, String requestId) {
        // 全量副本删除时，记录相关副本的展示时间，展示在日志中
        recordRelativeCopiesDeleteLog(requestId, deletableCopyList);
        // 针对每个副本更新用户配额
        deletableCopyList.forEach(
            copyId -> userQuotaManager.decreaseUsedQuota(requestId, copyRestApi.queryCopyByID(copyId)));
        // 删除关联副本
        log.debug("Delete relative copies, deletableCopyList: {}, requestId: {}",
            JSONArray.fromObject(deletableCopyList).toString(), requestId);
        copyRestApi.deleteCopiesForDatabase(deletableCopyList);
    }

    private void updateTaskCanStop(TaskCompleteMessageBo taskCompleteMessage) {
        JobStopParam jobStopParam = new JobStopParam();
        jobStopParam.setBackupEngineCancelable(true);
        jobStopParam.setEnforceStop(true);
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setEnableStop(true);
        jobCenterRestApi.initStopParam(updateJobRequest, jobStopParam);
        jobService.updateJob(taskCompleteMessage.getJobRequestId(), updateJobRequest);
    }

    /**
     * 适配器
     *
     * @param subType subType
     * @return true or false
     */
    @Override
    public boolean applicable(String subType) {
        List<String> jobTypes = Arrays.asList(
            JobTypeEnum.COPY_DELETE.getValue() + JobStatusLabelConstant.HYPHEN_STRING_DELIMITER + version(),
            JobTypeEnum.COPY_EXPIRE.getValue() + JobStatusLabelConstant.HYPHEN_STRING_DELIMITER + version());
        return jobTypes.contains(subType);
    }
}
