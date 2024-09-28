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
package openbackup.openstack.adapter.generator;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.JobScheduleDto;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.dto.SchedulePolicyDto;
import openbackup.openstack.adapter.dto.ScheduleRetentionDto;
import openbackup.openstack.adapter.enums.JobResult;
import openbackup.openstack.adapter.enums.JobScheduleType;
import openbackup.openstack.adapter.enums.OpenStackJobStatus;
import openbackup.openstack.adapter.enums.OpenStackJobType;
import openbackup.openstack.adapter.enums.ScheduleRetentionType;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.RetentionDto;
import com.huawei.oceanprotect.sla.sdk.dto.ScheduleDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import com.huawei.oceanprotect.sla.sdk.enums.Trigger;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.joda.time.DateTime;
import org.joda.time.format.DateTimeFormat;
import org.springframework.beans.BeanUtils;

import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * OpenStack相关实体类生成器
 *
 */
@Slf4j
public final class OpenStackModelsGenerator {
    private OpenStackModelsGenerator() {}

    /**
     * 生成OpenStackBackupJobDto
     *
     * @param protectedObject 受保护对象
     * @param sla SLA
     * @param job 保护对象最近一次任务
     * @return {@link OpenStackBackupJobDto} OpenStackBackupJobDto
     */
    public static OpenStackBackupJobDto generateBackupJob(ProtectedObjectInfo protectedObject, SlaDto sla, JobBo job) {
        OpenStackBackupJobDto backupJob = new OpenStackBackupJobDto();
        backupJob.setId(protectedObject.getResourceId());
        // 从保护对象高级参数提取数据
        Map<String, Object> extParameters = protectedObject.getExtParameters();
        backupJob.setName(extParameters.getOrDefault(OpenStackConstants.NAME, StringUtils.EMPTY).toString());
        backupJob.setDescription(
            extParameters.getOrDefault(OpenStackConstants.DESCRIPTION, StringUtils.EMPTY).toString());
        backupJob.setType(OpenStackJobType.create(
            extParameters.getOrDefault(OpenStackConstants.BACKUP_TYPE, StringUtils.EMPTY).toString()));
        backupJob.setInstanceId(
            extParameters.getOrDefault(OpenStackConstants.INSTANCE_ID, StringUtils.EMPTY).toString());
        // 从SLA高级参数提取数据
        PolicyDto policy = sla.getPolicyList().get(0);
        JsonNode slaExtParams = policy.getExtParameters();
        backupJob.setAutoRetryTimes(slaExtParams.get(SlaConstants.AUTO_RETRY_TIMES).asInt());
        backupJob.setAutoRetryWaitMinutes(slaExtParams.get(SlaConstants.AUTO_RETRY_WAIT_MINUTES).asInt());

        backupJob.setLastResult(
                job == null ? StringUtils.EMPTY : convert2LastResult(JobStatusEnum.get(job.getStatus())));
        backupJob.setStatus(convert2OpenStackJobStatus(ProtectionStatusEnum.getStatus(protectedObject.getStatus())));
        // 第一次创建备份任务时执行了手动备份，才会绑定global的SLA；执行手动备份不存在JobSchedule
        if (sla.isGlobal()) {
            log.info("Protected object: {} is bond a global sla: {}.", protectedObject.getResourceId(), sla.getUuid());
            return backupJob;
        }
        backupJob.setJobsSchedule(buildJobsSchedule(policy));
        return backupJob;
    }

    private static String convert2LastResult(JobStatusEnum status) {
        switch (status) {
            case SUCCESS:
                return JobResult.SUCCESS.getResult();
            case FAIL:
                return JobResult.FAIL.getResult();
            default:
                return JobResult.OTHERS.getResult();
        }
    }

    private static String convert2OpenStackJobStatus(ProtectionStatusEnum status) {
        switch (status) {
            case PROTECTED:
                return OpenStackJobStatus.RUNNING.getStatus();
            case UNPROTECTED:
                return OpenStackJobStatus.STOP.getStatus();
            default:
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "protection status is illegal.");
        }
    }

    private static JobScheduleDto buildJobsSchedule(PolicyDto policy) {
        JobScheduleDto jobSchedule = new JobScheduleDto();
        buildSchedulePolicy(policy.getSchedule(), jobSchedule);
        jobSchedule.setRetention(buildScheduleRetention(policy.getRetention()));
        return jobSchedule;
    }

    private static void buildSchedulePolicy(ScheduleDto slaSchedule, JobScheduleDto jobSchedule) {
        SchedulePolicyDto jobPolicy = new SchedulePolicyDto();
        if (Objects.equals(slaSchedule.getTrigger(), Trigger.INTERVAL)) {
            jobSchedule.setType(JobScheduleType.DAYS);
            jobPolicy.setIntervalDays(slaSchedule.getInterval());
        } else if (Objects.equals(slaSchedule.getTrigger(), Trigger.CUSTOMIZE_INTERVAL)) {
            jobSchedule.setType(JobScheduleType.WEEKS);
            jobPolicy.setDaysOfWeek(
                    slaSchedule.getDaysOfWeek().stream()
                            .map(week -> OpenStackConstants.WEEKS.indexOf(week) + 1)
                            .collect(Collectors.toList()));
        } else {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "sla trigger is illegal.");
        }
        jobPolicy.setStartDate(slaSchedule.getStartTime());
        jobPolicy.setStopDate(slaSchedule.getEndTime());
        jobPolicy.setExecuteTime(extractHourNumber(slaSchedule.getWindowStart()));
        jobSchedule.setPolicy(jobPolicy);
    }

    /**
     * 提取time字符串的Hour，转为数字
     *
     * @param time time
     * @return hour
     */
    private static Integer extractHourNumber(String time) {
        return DateTime.parse(time, DateTimeFormat.forPattern(DateFormatConstant.TIME)).getHourOfDay();
    }

    private static ScheduleRetentionDto buildScheduleRetention(RetentionDto slaRetention) {
        ScheduleRetentionDto jobRetention = new ScheduleRetentionDto();
        switch (slaRetention.getRetentionType()) {
            case PERMANENT:
                jobRetention.setType(ScheduleRetentionType.TIME);
                jobRetention.setCount(0);
                break;
            case TEMPORARY:
                jobRetention.setType(ScheduleRetentionType.TIME);
                jobRetention.setCount(slaRetention.getRetentionDuration());
                break;
            case QUANTITY:
                jobRetention.setType(ScheduleRetentionType.NUMBER);
                jobRetention.setCount(slaRetention.getRetentionQuantity());
                break;
            default:
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "sla retention type is illegal.");
        }
        return jobRetention;
    }

    /**
     * 生成OpenStackBackupJobDto响应值
     *
     * @param backupJob {@link OpenStackBackupJobDto} backupJob请求值
     * @param job 资源最近一次任务
     * @param resource 资源
     * @return {@link OpenStackBackupJobDto} backupJob响应值
     */
    public static OpenStackBackupJobDto generateBackupJobResp(
            OpenStackBackupJobDto backupJob, JobBo job, ProtectedResource resource) {
        OpenStackBackupJobDto resp = new OpenStackBackupJobDto();
        BeanUtils.copyProperties(backupJob, resp);
        resp.setId(resource.getUuid());
        resp.setLastResult(job == null ? StringUtils.EMPTY : convert2LastResult(JobStatusEnum.get(job.getStatus())));
        resp.setStatus(convert2OpenStackJobStatus(ProtectionStatusEnum.getStatus(resource.getProtectionStatus())));
        return resp;
    }

    /**
     * 生成OpenStackRestoreJobDto
     *
     * @param job {@link JobBo} 恢复任务
     * @return openStackRestoreJobDto
     */
    public static OpenStackRestoreJobDto generateRestoreJob(JobBo job) {
        OpenStackRestoreJobDto restoreJob = new OpenStackRestoreJobDto();

        restoreJob.setId(job.getJobId());
        restoreJob.setCopyId(job.getCopyId());

        JSONObject payload = JSONObject.fromObject(job.getMessage()).getJSONObject("payload");
        JSONObject advanceParams = payload.getJSONObject("advanceParams");
        restoreJob.setName(advanceParams.getString(OpenStackConstants.RESTORE_NAME));
        restoreJob.setDescription(advanceParams.getString(OpenStackConstants.DESCRIPTION));
        restoreJob.setType(OpenStackJobType.create(advanceParams.getString(OpenStackConstants.RESTORE_TYPE)));
        restoreJob.setInstanceId(advanceParams.getString(OpenStackConstants.INSTANCE_ID));

        restoreJob.setResult(convert2LastResult(JobStatusEnum.get(job.getStatus())));
        if (JobStatusEnum.FINISHED_STATUS_LIST.contains(JobStatusEnum.get(job.getStatus()))) {
            restoreJob.setStatus(OpenStackJobStatus.COMPLETED.getStatus());
        } else {
            restoreJob.setStatus(OpenStackJobStatus.RUNNING.getStatus());
        }

        return restoreJob;
    }
}
