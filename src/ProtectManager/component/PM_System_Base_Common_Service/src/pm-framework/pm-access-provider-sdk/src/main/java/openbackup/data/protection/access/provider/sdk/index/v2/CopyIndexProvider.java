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
package openbackup.data.protection.access.provider.sdk.index.v2;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 副本索引的provider,该类定义资源副本是否支持索引的接口。不用的应用根据需要创建
 *
 */
public interface CopyIndexProvider extends DataProtectionProvider<String> {
    /**
     * 资源是否支持对副本创建索引
     *
     * @return 支持返回true，不支持返回false
     */
    boolean isSupportIndex();
}
