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

import lombok.Data;

import java.util.Set;

/**
 * UserDetail from rest request
 *
 * @author dwx1009286
 * @version [OceanProtect 1.1.0]
 * @since 2022-02-24
 */
@Data
public class UserDetail {
    // 用户id
    private String userId;

    // 用户名
    private String userName;

    // 用户角色
    private Set<RoleInfo> rolesSet;

    // 用户类型
    private String userType;
}
