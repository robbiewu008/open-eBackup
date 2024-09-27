/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * dme cloud backup file system request
 *
 * @author g00500588
 * @since 2021/12/7
 */
@Data
public class DmeCloudBackupFileSystemRequest extends DmeArchiveRequest {
    @JsonProperty("MaxIncTimes")
    private int maxIncTimes;

    @JsonProperty("Aggregation")
    private boolean isAggregation;

    @JsonProperty("AutoCreateIndex")
    private boolean isAutoCreateIndex;

    @JsonProperty("FileSystemType")
    private int fileSystemType;

    @JsonProperty("ResourceName")
    private String resourceName;

    @JsonProperty("ResourceId")
    private String resourceId;
}
