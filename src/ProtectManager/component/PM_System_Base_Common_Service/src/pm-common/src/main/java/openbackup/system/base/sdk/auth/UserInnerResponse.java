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

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import openbackup.system.base.sdk.auth.model.RoleBo;

import java.util.Set;

/**
 * 功能描述
 *
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class UserInnerResponse {
    private String userId;

    private String userName;

    private boolean lock;

    private String description;

    private boolean sessionControl;

    private int sessionLimit;

    private String accessControl;

    private boolean defaultUser;

    private Set<RoleBo> rolesSet;

    private boolean sysAdmin;

    private String userType;

    private int loginType;

    private String dynamicCodeEmail;

    private boolean isNeverExpire;
}
