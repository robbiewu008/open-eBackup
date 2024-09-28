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

import java.util.Collections;
import java.util.List;

/**
 * 功能描述: 资源扫描 Provider
 *
 */
public interface ResourceScanProvider extends DataProtectionProvider<ProtectedEnvironment> {
    /**
     * 扫描受保护环境的资源
     *
     * @param environment 受保护环境
     * @return 受保护环境中的资源列表
     */
    List<ProtectedResource> scan(ProtectedEnvironment environment);

    /**
     * 当出现异常时，查询资源
     * <p>
     * DataProtectionAccessException或LegoCheckedException时触发
     *
     * @param uuid uuid
     * @return 受保护环境中的资源列表
     */
    default List<ProtectedResource> queryResourceWhenException(String uuid) {
        return Collections.emptyList();
    }
}
