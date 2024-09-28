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
package openbackup.data.access.framework.copy.mng.service;

import openbackup.system.base.sdk.copy.model.Copy;

import java.util.List;

/**
 * 功能描述
 *
 */
public interface CopyAuthVerifyService {
    /**
     * 校验副本查询权限
     *
     * @param copy 副本
     */
    void checkCopyQueryAuth(Copy copy);

    /**
     * 校验副本操作权限
     *
     * @param copy              副本
     * @param authOperationList 操作权限集合
     */
    void checkCopyOperationAuth(Copy copy, List<String> authOperationList);
}
