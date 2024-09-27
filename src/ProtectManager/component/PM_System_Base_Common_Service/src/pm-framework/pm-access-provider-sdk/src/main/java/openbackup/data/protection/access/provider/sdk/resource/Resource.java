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
package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;
import java.util.Map;

/**
 * Resource entity, this class entity Indicates a specific resource, for example db, file system, vm.
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class Resource {
    /**
     * FILE_SYSTEM
     */
    public static final String FILE_SYSTEM = "Fileset";

    /**
     * Oracle
     */
    public static final String ORACLE = "Oracle";

    /**
     * VMware
     */
    public static final String VMWARE = "VMware";

    private String uuid;

    private String name;

    private String type;

    @JsonProperty("sub_type")
    private String subType;

    private String path;

    @JsonProperty("parent_uuid")
    private String parentUuid;

    @JsonProperty("root_uuid")
    private String rootUuid;

    private boolean hasChildren;

    private String size;

    @JsonProperty("environment_name")
    private String environmentName;

    @JsonProperty("environment_endpoint")
    private String environmentEndpoint;

    @JsonProperty("instance_names")
    private String instanceNames;

    @JsonIgnore
    private List<Resource> children;

    // 爱数副本的gnsPath
    @JsonProperty("gns_path")
    private String gnsPath;

    private Map<String, String> extendInfo;
}
