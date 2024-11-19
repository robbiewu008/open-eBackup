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
package openbackup.system.base.sdk.auth.model.request;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.auth.model.HcsUserRegion;

import java.util.List;

/**
 * UserProjects对象属性
 *
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class HcsUserProjectInfo {
    /**
     * region信息
     */
    private List<HcsUserRegion> regions;

    private String id;

    private String name;

    private String iamProjectName;

    private String displayName;

    private String description;

    @JsonProperty("enabled")
    private boolean isEnabled;

    private String domainId;

    private String tenantId;

    private String tenantName;

    private String level;

    private String roleId;

    private String roleName;

    private String isShared;
}
