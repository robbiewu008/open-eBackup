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

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 虚拟机资源类
 *
 * @author t00482481
 * @since 2020-10-29
 */
@Data
public class VirtualResourceSchema extends ResourceEntity {
    @JsonProperty("vm_ip")
    private String vmIp;

    @JsonProperty("env_ip")
    private String envIp;

    @JsonProperty("link_status")
    private String linkStatus;

    @JsonProperty("mo_id")
    private String moId;

    @JsonProperty("alias_type")
    private String aliasType;

    @JsonProperty("alias_value")
    private String aliasValue;

    @JsonProperty("capacity")
    private String capacity;

    @JsonProperty("free_space")
    private String freeSpace;

    @JsonProperty("uncommitted")
    private String uncommitted;

    @JsonProperty("children")
    private String children;

    @JsonProperty("is_template")
    private String isTemplate;

    @JsonProperty("os_type")
    private String osType;

    @JsonProperty("partitions")
    private List<String> partitions;
}
