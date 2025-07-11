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
package openbackup.system.base.sdk.user;

import openbackup.system.base.sdk.user.model.RolePo;

/**
 * 角色sdk接口
 *
 */
public interface RoleServiceApi {
    /**
     * 通过用户id查询用户的默认角色
     *
     * @param userId 用户id
     * @return 默认角色id
     */
    RolePo getDefaultRolePoByUserId(String userId);
}
