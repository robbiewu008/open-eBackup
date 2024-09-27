/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.job;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Job消息基类
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-22
 */
public class TaskBaseMessage {
    @JsonProperty("job_request_id")
    private String jobRequestId;

    @JsonProperty("job_id")
    private String jobId;

    public String getJobRequestId() {
        return jobRequestId;
    }

    public void setJobRequestId(String jobRequestId) {
        this.jobRequestId = jobRequestId;
    }

    public String getJobId() {
        return jobId;
    }

    public void setJobId(String jobId) {
        this.jobId = jobId;
    }
}
