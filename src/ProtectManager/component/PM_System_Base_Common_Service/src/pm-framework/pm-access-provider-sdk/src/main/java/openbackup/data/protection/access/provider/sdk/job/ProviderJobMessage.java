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
package openbackup.data.protection.access.provider.sdk.job;

import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;

/**
 * job listener bo
 *
 */
public class ProviderJobMessage extends TaskBaseMessage {
    // 状态
    private ProviderJobStatusEnum status;

    @JsonProperty("job_speed")
    private String speed;

    @JsonProperty("job_progress")
    private Integer progress;

    @JsonProperty("job_logs")
    private List<ProviderJobLogMessage> jobLogs;

    public ProviderJobStatusEnum getStatus() {
        return status;
    }

    public void setStatus(ProviderJobStatusEnum status) {
        this.status = status;
    }

    public String getSpeed() {
        return speed;
    }

    public void setSpeed(String speed) {
        this.speed = speed;
    }

    public Integer getProgress() {
        return progress;
    }

    public void setProgress(Integer progress) {
        this.progress = progress;
    }

    public List<ProviderJobLogMessage> getJobLogs() {
        return jobLogs;
    }

    public void setJobLogs(List<ProviderJobLogMessage> jobLogs) {
        this.jobLogs = jobLogs;
    }
}
