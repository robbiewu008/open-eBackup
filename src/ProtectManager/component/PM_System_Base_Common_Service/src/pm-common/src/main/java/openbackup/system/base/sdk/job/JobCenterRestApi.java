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
package openbackup.system.base.sdk.job;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobBo;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.job.model.request.JobScheduleConfig;
import openbackup.system.base.sdk.job.model.request.JobStopParam;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * JobCenter Client Service
 *
 * @author h30003246
 * @since 2020-07-16
 */
@FeignClient(name = "JobCenterService", url = "${pm-system-base.url}/v1/internal/jobs",
    configuration = CommonFeignConfiguration.class)
public interface JobCenterRestApi {
    /**
     * 任务能否在pm侧强制停止
     */
    String ENFORCE_STOP = "enforce-stop";

    /**
     * 备份引擎是否能够取消任务
     */
    String BACKUP_ENGINE_CANCELABLE = "backup-engine-cancelable";

    /**
     * 创建任务
     *
     * @param job 任务
     * @return jobId 任务ID
     */
    @ExterAttack
    @PostMapping
    @ResponseBody
    String createJob(@RequestBody CreateJobRequest job);

    /**
     * Query jobs by types page list response.
     *
     * @param types the types
     * @param statusList the status list
     * @param startPage the start page
     * @param pageSize the page size
     * @return the page list response
     */
    @ExterAttack
    @GetMapping
    @ResponseBody
    PageListResponse<JobBo> queryJobsByTypes(@RequestParam List<String> types, @RequestParam List<String> statusList,
        @RequestParam int startPage, @RequestParam int pageSize);

    /**
     * 查询任务日志详情
     *
     * @param jobId 任务ID
     * @return 列表
     */
    @ExterAttack
    @GetMapping("{jobId}/logs?pageSize=1&orderBy=start_time&orderType=desc")
    PageListResponse<JobLogBo> queryLastJobLogs(@PathVariable("jobId") String jobId);

    /**
     * 查询任务列表
     *
     * @param jobId 任务ID
     * @param startPage 开始页码
     * @param pageSize 每页大小
     * @return 列表
     */
    @ExterAttack
    @GetMapping
    PageListResponse<JobBo> queryJobs(@RequestParam String jobId, @RequestParam int startPage,
        @RequestParam int pageSize);

    /**
     * 更新任务
     *
     * @param jobId job id
     * @param jobRequest jobRequest
     */
    @PutMapping("{jobId}/action/update")
    void updateJob(@PathVariable("jobId") String jobId, @RequestBody UpdateJobRequest jobRequest);

    /**
     * 批量更新任务
     *
     * @param jobBoList jobBo list
     */
    @PutMapping("/action/batchUpdate")
    void updateJobs(@RequestBody List<JobBo> jobBoList);

    /**
     * 删除任务
     *
     * @param jobId job id
     */
    @PutMapping("{jobId}/action/abort")
    void abortTask(@PathVariable("jobId") String jobId);

    /**
     * verify job ownership
     *
     * @param userId user id
     * @param uuidList uuid list
     */
    @GetMapping("/action/verify")
    void verifyJobOwnership(@RequestParam("user_id") String userId, @RequestParam("uuid_list") List<String> uuidList);

    /**
     * 系统数据备份恢复后更新未完成任务状态
     */
    @PutMapping("/running/action/reset")
    void updateUnfinishedJob();

    /**
     * complete job
     *
     * @param jobId job id
     * @param status status
     */
    default void completeJob(String jobId, JobStatusEnum status) {
        completeJob(jobId, status, false);
    }

    /**
     * complete job
     *
     * @param jobId job id
     * @param status status
     * @param isUpdateProgress isUpdateProgress
     */
    default void completeJob(String jobId, JobStatusEnum status, boolean isUpdateProgress) {
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        if (status == JobStatusEnum.SUCCESS || isUpdateProgress) {
            updateJobRequest.setProgress(IsmNumberConstant.HUNDRED);
        }
        updateJobRequest.setStatus(status);
        updateJob(jobId, updateJobRequest);
    }

    /**
     * update job schedule policy
     *
     * @param config config
     */
    @PutMapping("/action/update-schedule-policy")
    void updateJobSchedulePolicy(@RequestBody JobScheduleConfig config);

    /**
     * disable enable stop
     *
     * @param jobId job id
     */
    default void disableEnableStop(String jobId) {
        UpdateJobRequest request = new UpdateJobRequest();
        request.setEnableStop(false);
        updateJob(jobId, request);
    }

    /**
     * enforce job stop
     *
     * @param jobId job id
     * @param canEnforceStop enforce
     */
    default void enforceStop(String jobId, boolean canEnforceStop) {
        UpdateJobRequest request = new UpdateJobRequest();
        initEnforceStop(request, canEnforceStop);
        updateJob(jobId, request);
    }

    /**
     * enforce job stop
     *
     * @param request request
     * @param canEnforceStop enforce stop
     */
    default void initEnforceStop(UpdateJobRequest request, boolean canEnforceStop) {
        request.setData(new JSONObject().set(ENFORCE_STOP, canEnforceStop));
    }

    /**
     * enforce job stop
     *
     * @param jobId job id
     * @param jobRequest job request
     * @param canEnforceStop enforce stop
     */
    default void updateJob(String jobId, @RequestBody UpdateJobRequest jobRequest, boolean canEnforceStop) {
        initEnforceStop(jobRequest, canEnforceStop);
        updateJob(jobId, jobRequest);
    }

    /**
     * update job
     *
     * @param jobId job id
     * @param jobRequest update job request
     * @param stopParam update job stopParam
     */
    default void updateJob(String jobId, @RequestBody UpdateJobRequest jobRequest, JobStopParam stopParam) {
        initStopParam(jobRequest, stopParam);
        updateJob(jobId, jobRequest);
    }

    /**
     * initStopParam
     *
     * @param request update job request
     * @param jobStopParam update job stop param
     */
    default void initStopParam(UpdateJobRequest request, JobStopParam jobStopParam) {
        request.setData(new JSONObject().set(BACKUP_ENGINE_CANCELABLE, jobStopParam.isBackupEngineCancelable())
            .set(ENFORCE_STOP, jobStopParam.isEnforceStop()));
    }
}
