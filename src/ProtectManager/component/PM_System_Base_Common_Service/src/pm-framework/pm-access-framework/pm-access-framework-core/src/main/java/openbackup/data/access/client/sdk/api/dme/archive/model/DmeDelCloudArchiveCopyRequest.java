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
 * DME Delete Cloud Archive request
 *
 * @author d00512967
 * @since 2020-12-12
 */
@Data
public class DmeDelCloudArchiveCopyRequest {
    @JsonProperty("RequestId")
    private String requestId;

    @JsonProperty("TaskID")
    private String taskId;

    @JsonProperty("ArchiveCopyId")
    private String archiveCopyId;

    @JsonProperty("Type")
    private Integer type;

    @JsonProperty("Cloud")
    private CloudArchiveInfo cloudArchiveInfo;

    @JsonProperty("ApplicationType")
    private Integer applicationType;

    // 0: 非强制 1:强制删除
    @JsonProperty("IsForce")
    private int isForce;
}
