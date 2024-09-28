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
package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * dme cloud backup file system request
 *
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
