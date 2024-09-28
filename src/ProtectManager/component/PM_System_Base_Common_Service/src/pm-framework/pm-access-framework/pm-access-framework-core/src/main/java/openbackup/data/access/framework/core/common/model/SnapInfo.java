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
package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * Global Search index
 *
 **/
@Getter
@Setter
public class SnapInfo {
    @JsonProperty("snap_id")
    private String snapId;

    @JsonProperty("snap_type")
    private String snapType;

    @JsonProperty("timestamp")
    private String timestamp;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("resource_name")
    private String resourceName;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("chain_id")
    private String chainId;

    @JsonProperty("gn")
    private int gn;

    @JsonProperty("snap_metadata")
    private String snapMetadata;

    @JsonProperty("user_id")
    private String userId;

    @JsonProperty("esn")
    private String esn;
}
