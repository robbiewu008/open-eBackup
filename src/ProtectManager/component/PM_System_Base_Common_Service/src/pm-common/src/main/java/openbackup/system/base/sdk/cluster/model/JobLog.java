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
package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 任务进度
 *
 * @author w00607005
 * @since 2023-05-23
 */
@Data
public class JobLog {
    @JsonProperty("log_info")
    private String logInfo;

    @JsonProperty("log_info_param")
    private List<String> logInfoParam;

    @JsonProperty("log_timestamp")
    private Long logTimestamp;

    @JsonProperty("log_detail")
    private Integer logDetail;

    @JsonProperty("log_detail_param")
    private List<String> logDetailParam;

    @JsonProperty("log_detail_info")
    private List<String> logDetailInfo;

    @JsonProperty("log_level")
    private Integer logLevel;

    @JsonProperty("unique")
    private boolean isUnique;
}
