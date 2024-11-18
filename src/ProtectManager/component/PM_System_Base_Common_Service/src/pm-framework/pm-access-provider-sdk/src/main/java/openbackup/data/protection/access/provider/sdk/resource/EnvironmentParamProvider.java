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
 * 功能描述: EnvironmentParamProvider
 *
 */
public interface EnvironmentParamProvider extends DataProtectionProvider<ProtectedEnvironment> {
    /**
     * 注册、修改受保护环境时，在下发agent插件之前，先校验、填充参数
     *
     * @param environment 受保护环境
     */
    void checkAndPrepareParam(ProtectedEnvironment environment);

    /**
     * 连通性检查之后，某些特性需要从agent查询信息并回填到environment中
     *
     * @param environment 受保护环境
     */
    void updateEnvironment(ProtectedEnvironment environment);

    /**
     * 校验受保护环境是否被重复注册
     *
     * @param environment 受保护环境
     */
    void checkEnvironmentRepeat(ProtectedEnvironment environment);
}