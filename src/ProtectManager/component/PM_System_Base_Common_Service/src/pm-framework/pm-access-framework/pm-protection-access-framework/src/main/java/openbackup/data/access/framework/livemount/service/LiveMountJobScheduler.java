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
package openbackup.data.access.framework.livemount.service;

import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.dto.DeliverTaskReq;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.collections.MapUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * 结束安全一体机即时挂载的超时任务
 *
 */
@Slf4j
@Component
@EnableScheduling
public class LiveMountJobScheduler {
    private static final long INITIAL_DELAY = 1000L * 60L * 2L;

    private static final long FIXED_RATE = 1000L * 60L * 10L;

    private static final long ONE_HOUR = 1000L * 60L * 60L;

    @Autowired
    private AgentBusinessService agentBusinessService;

    @Autowired
    private JobService jobService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private DeeLiveMountRestApi deeLiveMountRestApi;

    /**
     * 每隔10分钟，结束安全一体机即时挂载的超时任务。
     */
    @Scheduled(initialDelay = INITIAL_DELAY, fixedRate = FIXED_RATE)
    public void regularUpdateTaskStatus() {
        if (deployTypeService.isCyberEngine()) {
            return;
        }
        // 过滤条件
        QueryJobRequest conditions = new QueryJobRequest();
        conditions.setTypes(Collections.singletonList(JobTypeEnum.LIVE_MOUNT.getValue()));
        conditions.setStatusList(JobStatusEnum.LONG_TIME_JOB_STOP_STATUS_LIST.stream()
            .map(JobStatusEnum::name)
            .collect(Collectors.toList()));
        PageListResponse<JobBo> jobs = jobService.queryJobs(conditions, new PagingParamRequest());
        jobs.getRecords().stream().forEach(jobBo -> {
            String message = jobBo.getMessage();
            JobMessage jobMessage = JSONObject.toBean(message, JobMessage.class);
            JSONObject payload = jobMessage.getPayload();
            LiveMountEntity liveMountEntity = payload.getBean("live_mount", LiveMountEntity.class);
            JSONObject parameters = JSONObject.fromObject(liveMountEntity.getParameters());
            JSONObject performance = parameters.getJSONObject("performance");
            Map<String, Object> params = performance.toMap(Object.class);
            Integer fileSystemKeepTime = MapUtils.getInteger(params, "fileSystemKeepTime");
            if (!Objects.isNull(fileSystemKeepTime)) {
                long finalTime = jobBo.getStartTime() + fileSystemKeepTime.intValue() * ONE_HOUR;
                if (System.currentTimeMillis() > finalTime) {
                    DeliverTaskReq deliverTaskReq = new DeliverTaskReq();
                    deliverTaskReq.setTaskId(jobBo.getJobId());
                    deliverTaskReq.setStatus("failed");
                    agentBusinessService.deliverTaskStatus(deliverTaskReq);
                }
            }
        });
    }
}