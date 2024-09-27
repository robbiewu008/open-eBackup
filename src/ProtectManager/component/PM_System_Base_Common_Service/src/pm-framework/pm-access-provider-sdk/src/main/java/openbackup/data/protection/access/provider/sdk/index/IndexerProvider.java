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
package openbackup.data.protection.access.provider.sdk.index;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;

/**
 * 生成索引文件
 *
 * @author t00482481
 * @since 2020-08-26
 */
public interface IndexerProvider extends DataProtectionProvider<String> {
    /**
     * 生成索引文件
     *
     * @param requestId 请求id
     * @param copy 副本信息
     * @return 是否成功
     */
    boolean generateIndexFile(String requestId, CopyBo copy);
}
