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
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;

/**
 * 主机环境上资源浏览 Provider
 *
 */
public interface ResourceBrowseProvider extends DataProtectionProvider<String> {
    /**
     * 浏览环境资源
     *
     * @param environment 受保护环境
     * @param environmentConditions 查询资源的条件
     * @return 返回资源列表
     */
    PageListResponse<ProtectedResource> browse(
        ProtectedEnvironment environment, BrowseEnvironmentResourceConditions environmentConditions);
}
