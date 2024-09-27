/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.access.framework.protection.common.converters;

import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.protection.controller.req.UpdateJobLogRequest;
import openbackup.data.access.framework.protection.controller.req.UpdateJobStatusRequest;
import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import com.huawei.oceanprotect.job.dto.JobLogDto;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.JobAdditionalStatusEnum;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.JobSpeedConverter;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobLogMessageLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * JobStatus数据转换类
 *
 * @author w00616953
 * @since 2021-12-09
 */
@Slf4j
public final class JobDataConverter {
    private static final String LOG_DETAIL_INFO_SEPARATOR = "<br>";

    private JobDataConverter() {
    }

    /**
     * 将DME上报的JobStatus数据模型，转换为pm数据库对应字段的数据模型
     *
     * @param statusRequest DME上报的数据模型
     * @return pm数据库对应字段的数据模型
     */
    public static UpdateJobRequest convertStatusRequest2UpdateJobRequest(UpdateJobStatusRequest statusRequest) {
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setProgress(statusRequest.getProgress());

        ProviderJobStatusEnum status = convertToProviderJobStatus(statusRequest.getStatus());
        updateJobRequest.setStatus(JobStatusEnum.get(status.name()));

        // 上报的附加状态属于已定义的附加状态的值，才更新到数据库中
        if (JobAdditionalStatusEnum.getEnum(statusRequest.getAdditionalStatus()) != null) {
            updateJobRequest.setAdditionalStatus(statusRequest.getAdditionalStatus());
        }

        String dmeSpeed = String.valueOf(statusRequest.getSpeed());
        if (!VerifyUtil.isEmpty(dmeSpeed)) {
            String jobSpeed = JobSpeedConverter.convertJobSpeed(dmeSpeed);
            updateJobRequest.setSpeed(jobSpeed);
        }

        Object extendField = statusRequest.getExtendField();
        if (!VerifyUtil.isEmpty(extendField)) {
            JSONObject extendFieldJson = JSONObject.fromObject(extendField);
            updateJobRequest.setExtendStr(extendFieldJson.toString());
        }
        if (!VerifyUtil.isEmpty(statusRequest.getTargetLocation())) {
            updateJobRequest.setTargetLocation(statusRequest.getTargetLocation());
        }
        if (!VerifyUtil.isEmpty(statusRequest.getTargetName())) {
            updateJobRequest.setTargetName(statusRequest.getTargetName());
        }

        return updateJobRequest;
    }

    /**
     * 获取dme上报的任务日志信息
     *
     * @param jobId 任务ID
     * @param jobStatusRequest DME上报更新任务的数据
     * @return 任务日志
     */
    public static List<JobLogDto> getJobLogsFromStatusRequest(String jobId, UpdateJobStatusRequest jobStatusRequest) {
        List<UpdateJobLogRequest> jobLogRequestList = jobStatusRequest.getJobLogs();

        if (VerifyUtil.isEmpty(jobLogRequestList)) {
            return Collections.emptyList();
        }

        return jobLogRequestList.stream().map(jobLogRequest -> {
            JobLogDto jobLog = JobDataConverter.convertJobLogRequestToJobLog(jobLogRequest);
            jobLog.setJobId(jobId);
            return jobLog;
        }).collect(Collectors.toList());
    }

    /**
     * 将DME上报的JobLog数据模型，转换为pm数据库对应字段的数据模型
     *
     * @param jobLogRequest DME上报的JobLog数据模型
     * @return pm数据库对应字段的数据模型
     */
    private static JobLogDto convertJobLogRequestToJobLog(UpdateJobLogRequest jobLogRequest) {
        JobLogDto jobLog = new JobLogDto();

        jobLog.setUnique(jobLogRequest.isUnique());
        jobLog.setLogInfo(jobLogRequest.getLogInfo());
        Long logDetail = jobLogRequest.getLogDetail();
        if (!VerifyUtil.isEmpty(logDetail)) {
            jobLog.setLogDetail(String.valueOf(logDetail));
        }

        // 日志错误详情列表按<br>拼接为字符串入库
        String logDetailInfo = StringUtils.join(jobLogRequest.getLogDetailInfo(), LOG_DETAIL_INFO_SEPARATOR);
        jobLog.setLogDetailInfo(logDetailInfo);

        // 如果传入的时间长度为10，统一转为13位
        if (!VerifyUtil.isEmpty(jobLogRequest.getLogTimestamp())) {
            Long logTimestamp = jobLogRequest.getLogTimestamp();
            if (logTimestamp.toString().length() == IsmNumberConstant.TEN) {
                logTimestamp *= IsmNumberConstant.THOUSAND;
            }
            jobLog.setStartTime(logTimestamp);
        }

        // 将dme日志级别转换为pm日志级别
        if (!VerifyUtil.isEmpty(jobLogRequest.getLogLevel())) {
            JobLogLevelEnum jobLogLevelEnum = JobLogMessageLevelEnum.getJogLogStatus(jobLogRequest.getLogLevel());
            jobLog.setLevel(jobLogLevelEnum.getValue());
        }

        if (!VerifyUtil.isEmpty(jobLogRequest.getLogInfoParam())) {
            jobLog.setLogInfoParam(JSONArray.fromObject(jobLogRequest.getLogInfoParam()).toString());
        }
        if (!VerifyUtil.isEmpty(jobLogRequest.getLogDetailParam())) {
            jobLog.setLogDetailParam(JSONArray.fromObject(jobLogRequest.getLogDetailParam()).toString());
        }

        return jobLog;
    }

    /**
     * 将dme上报任务的请求数据，转换为监听任务完成使用的数据
     *
     * @param job 任务
     * @param updateRequest dme上报任务的请求数据
     * @return 人挺任务完成使用的数据
     */
    public static TaskCompleteMessageDto convertJobStatusRequestToTaskCompleteDto(
            JobBo job, UpdateJobStatusRequest updateRequest) {
        TaskCompleteMessageDto messageDto = new TaskCompleteMessageDto();
        String jobType = job.getType();
        if (!(ResourceSubTypeEnum.VMWARE.getType().equals(job.getSourceSubType())
                && JobTypeEnum.ARCHIVE.getValue().equals(job.getType()))) {
            jobType = jobType + "-" + UnifiedTaskCompleteHandler.V2;
        }
        messageDto.setJobType(jobType);
        messageDto.setJobStatus(updateRequest.getStatus());
        messageDto.setJobId(updateRequest.getJobRequestId());
        messageDto.setJobRequestId(updateRequest.getJobRequestId());
        messageDto.setTaskId(updateRequest.getTaskId());
        messageDto.setJobProgress(updateRequest.getProgress());
        JSONObject extendsInfo;
        if (!VerifyUtil.isEmpty(updateRequest.getExtendField())) {
            extendsInfo = JSONObject.fromObject(updateRequest.getExtendField());
        } else {
            extendsInfo = new JSONObject();
        }
        extendsInfo.set("taskId", updateRequest.getTaskId());
        log.info("job complete: id={}, taskId={}, extendInfo={}", updateRequest.getJobRequestId(),
                updateRequest.getTaskId(), extendsInfo);
        messageDto.setExtendsInfo(extendsInfo);
        messageDto.setSpeed(updateRequest.getSpeed());
        return messageDto;
    }

    /**
     * 将dme上报的任务状态转化为pm的任务状态
     *
     * @param value dme上报的任务状态值
     * @return 对应的pm的任务状态
     */
    public static ProviderJobStatusEnum convertToProviderJobStatus(int value) {
        DmeJobStatusEnum dmeTaskStatus = DmeJobStatusEnum.fromStatus(value);
        switch (dmeTaskStatus) {
            case RUNNING:
                return ProviderJobStatusEnum.RUNNING;
            case ABORTING:
                return ProviderJobStatusEnum.ABORTING;
            case SUCCESS:
                return ProviderJobStatusEnum.SUCCESS;
            case PARTIAL_SUCCESS:
                return ProviderJobStatusEnum.PARTIAL_SUCCESS;
            case ABORTED:
                return ProviderJobStatusEnum.ABORTED;
            case ABORTED_FAILED:
                return ProviderJobStatusEnum.ABORT_FAILED;
            default:
                return ProviderJobStatusEnum.FAIL;
        }
    }
}
