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
package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * hcs资源集
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/26
 */
@Getter
@Setter
public class HcsProject {
    private String id;
    private String name;
    private String description;
    @JsonProperty("domain_id")
    private String domainId;
    private boolean enabled;
    @JsonProperty("tenant_id")
    private String tenantId;
    @JsonProperty("is_shared")
    private boolean isShared;
    @JsonProperty("tenant_name")
    private String tenantName;
    @JsonProperty("create_user_id")
    private String createUserId;
    @JsonProperty("create_user_name")
    private String createUserName;
    private List<Region> regions;
}
