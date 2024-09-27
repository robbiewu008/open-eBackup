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
package openbackup.system.base.sdk.auth;

import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.sdk.auth.model.ResourceSetAuthorization;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import org.apache.logging.log4j.util.Strings;

import java.util.HashSet;
import java.util.Set;

/**
 * 功能描述
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-11-26
 */
@Getter
@Setter
public class UserRequest {
    private String userId;

    private String userName;

    private String userPassword;

    private String confirmPassword;

    private String description = Strings.EMPTY;

    private Set<String> rolesIdsSet;

    @JsonProperty("sessionControl")
    private boolean isSessionControl;

    private Set<ResourceSetAuthorization> resourceSetAuthorizationSets = new HashSet<>();

    private int sessionLimit = 1;

    private String userType = UserTypeEnum.COMMON.getValue();

    private int loginType;

    private String dynamicCodeEmail = Strings.EMPTY;
}
