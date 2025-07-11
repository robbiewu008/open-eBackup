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
 * 扫描归档副本 请求体
 *
 */
@Data
public class DmeScanArchiveRequestRequest {
    @JsonProperty("TaskID")
    String taskId;

    @JsonProperty("RequestID")
    String requestId;

    @JsonProperty("ArchiveStorage")
    ArchiveStorage archiveStorage;

    @JsonProperty("BackupStorage")
    BackupStorage backupStorage;
}
