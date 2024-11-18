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
package openbackup.data.protection.access.provider.sdk.resourcegroup.dto;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * 资源组保护对象
 *
 */

@Getter
@Setter
public class ResourceGroupProtectedObjectDto {
    private String uuid;

    private String slaId;

    private String slaName;

    private String name;

    private String envId;

    private String envType;

    private String resourceId;

    private String resourceGroupId;

    private String type;

    private String subType;

    @JsonProperty("slaCompliance")
    private Boolean isSlaCompliance;

    private String path;

    private JSONObject extParameters;

    private int status;

    private Timestamp latestTime;

    private Timestamp earliestTime;

    private String chainId;

    private String consistentStatus;

    private String consistentResults;
}