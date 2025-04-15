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
package openbackup.data.access.framework.protection.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.common.utils.JSONObject;

/**
 * dme task entity
 *
 */
@Data
public class TaskCompleteMessageDto {
    @JsonProperty("job_request_id")
    private String jobRequestId;

    @JsonProperty("job_id")
    private String jobId;

    private String taskId;

    @JsonProperty("job_status")
    private Integer jobStatus;

    @JsonProperty("job_progress")
    private Integer jobProgress;

    @JsonProperty("extend_info")
    private JSONObject extendsInfo;

    @JsonProperty("additional_status")
    private String additionalStatus;

    private String jobType;

    private Long speed;

    /**
     * get extend info
     *
     * @param type type
     * @param <T> template type
     * @return result
     */
    public <T> T getExtendsInfo(Class<T> type) {
        return getExtendsInfo(type, null);
    }

    /**
     * get extend info
     *
     * @param type type
     * @param defaultInfo default info
     * @param <T> template type
     * @return result
     */
    public <T> T getExtendsInfo(Class<T> type, T defaultInfo) {
        if (extendsInfo == null) {
            return defaultInfo;
        }
        return extendsInfo.toBean(type);
    }
}
