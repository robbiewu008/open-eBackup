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
 * DME token
 *
 * @author z30062305
 * @since 2023-07-027
 */
@Setter
@Getter
public class DmeToken {
    private String tokenStr;

    private List<String> methods;

    @JsonProperty("expires_at")
    private String expiresAt;

    private String issuedAt;

    private TokenUser user;

    private Domain domain;

    private List<CataLog> catalog;

    private List<Role> roles;

    private Project project;
}
