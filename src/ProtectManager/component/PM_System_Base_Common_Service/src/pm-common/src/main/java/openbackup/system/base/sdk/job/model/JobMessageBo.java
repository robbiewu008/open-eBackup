/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.job.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * job listener bo
 *
 * @author h30003246
 * @version [OceanProtect 8.1.RC1]
 * @since 2020-04-01
 */
@Data
public class JobMessageBo {
    // 状态
    private JobStatusEnum status;

    @JsonProperty("additional_status")
    private String additionalStatus;

    @JsonProperty("job_request_id")
    private String jobRequestId;

    @JsonProperty("job_id")
    private String jobId;

    @JsonProperty("job_speed")
    private String speed;

    @JsonProperty("job_progress")
    private Integer progress;

    @JsonProperty("extend_info")
    private Object extendStr;

    @JsonProperty("job_logs")
    private List<JobLogMessageBo> jobLogs;
}
