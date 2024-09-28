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

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 环境删除provider
 * 根据resourceSubType区分
 *
 */
public interface ProtectedEnvironmentDeleteProvider extends DataProtectionProvider<String> {
    /**
     * 删除时检查：前端调用时检查
     * 删除environment时调用
     *
     * @param env 资源
     * @return 检查结果，true：继续执行删除，false：终止删除，直接return
     */
    default boolean frontCheck(ProtectedEnvironment env) {
        return true;
    }

    /**
     * 删除时检查：通用检查
     * 删除environment时调用
     *
     * @param env 资源
     * @return 检查结果，true：继续执行删除，false：终止删除，直接return
     */
    default boolean check(ProtectedEnvironment env) {
        return true;
    }

    /**
     * 删除后处理
     *
     * @param env 资源id
     */
    default void afterDelete(ProtectedEnvironment env) {
    }
}
