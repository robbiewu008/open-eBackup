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
 * 更新归档副本PM元数据请求体
 *
 * @author z30009433
 * @since 2020-12-30
 */
@Data
public class DmeArchiveUpdateSnapRequest {
    @JsonProperty("Id")
    String archiveId;

    @JsonProperty("PMMetadata")
    String metadata;

    @JsonProperty("Type")
    int type;

    @JsonProperty("Cloud")
    CloudArchiveInfo cloud;
}
