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
package openbackup.system.base.common.model.job;

import com.fasterxml.jackson.annotation.JsonProperty;

import openbackup.system.base.common.utils.JSONObject;

import lombok.Data;

/**
 * 任务重试payload参数
 *
 **/
@Data
public class JobPayload {
    @JsonProperty("execute_type")
    private String executeType;

    @JsonProperty("chain_id")
    private String chainId;

    @JsonProperty("auto_retry")
    private Boolean autoRetry;

    @JsonProperty("auto_retry_wait_minutes")
    private Integer autoRetryWaitMinutes;

    @JsonProperty("auto_retry_times")
    private Integer autoRetryTimes;

    private JSONObject policy;
}
