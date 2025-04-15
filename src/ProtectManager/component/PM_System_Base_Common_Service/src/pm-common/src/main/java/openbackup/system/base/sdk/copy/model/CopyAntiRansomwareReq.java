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
package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Date;

/**
 * 防勒索检测入参对象
 *
 */
@Data
public class CopyAntiRansomwareReq {
    @JsonProperty("copy_id")
    private String copyId;

    private Integer status;

    private String model;

    @JsonProperty("detection_start_time")
    private Date detectionStartTime;

    @JsonProperty("detection_end_time")
    private Date detectionEndTime;

    private String report;

    @JsonProperty("tenant_id")
    private String tenantId;

    @JsonProperty("tenant_name")
    private String tenantName;
}