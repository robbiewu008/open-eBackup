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
package openbackup.data.protection.access.provider.sdk.repository;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 存储单元组修改校验扩展接口
 *
 **/
public interface StorageUnitGroupCheckProvider extends DataProtectionProvider<String> {
    /**
     * 存储单元组修改自定义校验逻辑
     *
     * @return false
     */
    default boolean isSupportParallelStorage() {
        return false;
    }
}
