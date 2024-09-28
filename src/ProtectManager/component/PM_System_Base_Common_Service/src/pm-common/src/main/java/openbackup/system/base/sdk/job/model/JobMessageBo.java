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
package openbackup.system.base.sdk.job.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * job listener bo
 *
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
