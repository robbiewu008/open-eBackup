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
package openbackup.data.protection.access.provider.sdk.livemount;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Performance
 *
 */
@Data
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
public class LiveMountPerformance {
    @JsonProperty("BandwidthMin")
    @JsonAlias("min_bandwidth")
    private int bandwidthMin;

    @JsonProperty("BandwidthMax")
    @JsonAlias("max_bandwidth")
    private int bandwidthMax;

    @JsonProperty("BandwidthBurst")
    @JsonAlias("burst_bandwidth")
    private int bandwidthBurst;

    @JsonProperty("IOPSMin")
    @JsonAlias("min_iops")
    private int iopsMin;

    @JsonProperty("IOPSMax")
    @JsonAlias("max_iops")
    private int iopsMax;

    @JsonProperty("IOPSBurst")
    @JsonAlias("burst_iops")
    private int iopsBurst;

    @JsonProperty("BurstTime")
    @JsonAlias("burst_time")
    private int burstTime;

    @JsonProperty("Latency")
    @JsonAlias("latency")
    private int latency;

    private int fileSystemKeepTime;
}
