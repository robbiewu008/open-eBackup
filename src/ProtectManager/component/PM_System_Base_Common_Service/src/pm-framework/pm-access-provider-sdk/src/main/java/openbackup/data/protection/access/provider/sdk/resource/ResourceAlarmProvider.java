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
 * 受保护资源发送告警资源名称填充Provider
 *
 */
public interface ResourceAlarmProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 获取资源名称
     *
     * @param resource 资源
     * @return String 默认发资源名称，插件自定义资源名称的返回值
     */
    default String getAlarmResourceName(ProtectedResource resource) {
        return resource.getName();
    }
}
