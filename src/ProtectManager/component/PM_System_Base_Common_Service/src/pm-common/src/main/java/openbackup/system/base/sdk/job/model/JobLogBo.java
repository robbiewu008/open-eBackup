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
 * 任务日志
 *
 * @author y00407642
 * @version [8.0.1]
 * @since 2020-06-12
 */
@Data
public class JobLogBo {
    private String jobId;

    private Long startTime;

    private Long endTime;

    private String logInfo;

    private String level;

    private List<String> logInfoParam;

    private String logDetail;

    private List<String> logDetailParam;

    private String logDetailInfo;

    /**
     * 该条日志是否唯一
     */
    @JsonProperty("unique")
    private boolean isUnique;
}
