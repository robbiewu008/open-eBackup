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
package openbackup.system.base.sdk.resource.model;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 文件集
 *
 */
@Data
public class ResourceEntity {
    /**
     * RESOURCE_ONLINE
     */
    public static final String RESOURCE_ONLINE = "0";

    private String uuid;

    private String name;

    private String type;

    @JsonProperty("sub_type")
    private String subType;

    private String path;

    @JsonProperty("created_time")
    private String createdTime;

    @JsonProperty("ext_parameters")
    private JSONObject extParameters;

    @JsonProperty("parent_name")
    private String parentName;

    @JsonProperty("parent_uuid")
    private String parentUuid;

    @JsonProperty("root_uuid")
    private String rootUuid;

    @JsonProperty("environment_name")
    private String environmentName;

    @JsonProperty("environment_uuid")
    private String environmentUuid;

    @JsonProperty("environment_endpoint")
    private String environmentEndPoint;

    @JsonProperty("environment_os_type")
    private String environmentOsType;

    @JsonProperty("environment_type")
    private String environmentType;

    @JsonProperty("environment_sub_type")
    private String environmentSubType;

    @JsonProperty("environment_is_cluster")
    private boolean environmentIsCluster;

    @JsonProperty("environment_os_name")
    private String environmentOsName;

    @JsonProperty("children_uuids")
    private List<String> childrenUuids;

    @JsonProperty("authorized_user")
    private String authorizedUser;

    @JsonProperty("user_id")
    private String userId;

    private String version;
}
