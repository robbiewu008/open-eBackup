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
package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 功能描述: FileSystemScrubResponse
 *
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class FileSystemScrubResponse {
    @JsonProperty("file_system_id")
    private String fsId;

    @JsonProperty("running_status")
    private String runningStatus;

    @JsonProperty("scan_range")
    private String range;

    @JsonProperty("medium_errors")
    private long mediumErrors;

    @JsonProperty("other_errors")
    private long otherErrors;

    @JsonProperty("process")
    private long process;

    @JsonProperty("total_scaned_capacity")
    private long totalScanedCapacity;

    @JsonProperty("scrubing_speed")
    private long scrubingSpeed;

    @JsonProperty("start_time")
    private String startTime;

    @JsonProperty("end_time")
    private String endTime;
}
