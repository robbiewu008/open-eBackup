/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
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
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-27
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