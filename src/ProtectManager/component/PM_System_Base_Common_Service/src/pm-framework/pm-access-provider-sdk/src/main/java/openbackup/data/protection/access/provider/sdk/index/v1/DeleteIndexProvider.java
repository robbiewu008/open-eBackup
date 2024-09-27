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
package openbackup.data.protection.access.provider.sdk.index.v1;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 索引删除Provider(v1版本)
 *
 * @author g30003063
 * @since 2022-01-24
 */
public interface DeleteIndexProvider extends DataProtectionProvider<String> {
    /**
     * 删除索引
     *
     * @param requestId 请求ID
     * @param copyId 副本ID
     */
    void deleteIndex(String requestId, String copyId);
}