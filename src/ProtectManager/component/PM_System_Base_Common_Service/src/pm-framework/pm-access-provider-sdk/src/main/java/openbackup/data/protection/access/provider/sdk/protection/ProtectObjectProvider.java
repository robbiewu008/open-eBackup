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
package openbackup.data.protection.access.provider.sdk.protection;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.protection.model.CheckProtectObjectDto;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * 保护Provider, 根据subType区分
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/10
 */
public interface ProtectObjectProvider extends DataProtectionProvider<String> {
    /**
     * 创建保护前
     *
     * @param checkProtectObjectDto checkProtectObjectDto
     */
    void beforeCreate(CheckProtectObjectDto checkProtectObjectDto);

    /**
     * 修改保护前
     *
     * @param checkProtectObjectDto checkProtectObjectDto
     */
    void beforeUpdate(CheckProtectObjectDto checkProtectObjectDto);

    /**
     * 创建/修改等失败
     *
     * @param checkProtectObjectDto checkProtectObjectDto
     */
    void failedOnCreateOrUpdate(CheckProtectObjectDto checkProtectObjectDto);

    /**
     * 移除保护
     *
     * @param protectedResource protectedResource
     */
    void remove(ProtectedResource protectedResource);
}
