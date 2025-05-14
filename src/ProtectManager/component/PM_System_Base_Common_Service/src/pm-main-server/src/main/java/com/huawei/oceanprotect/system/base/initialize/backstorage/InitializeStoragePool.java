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
package com.huawei.oceanprotect.system.base.initialize.backstorage;

import com.huawei.oceanprotect.system.base.initialize.backstorage.beans.InitBackActionResult;

import openbackup.system.base.common.model.repository.StoragePool;

/**
 * 初始化存储池
 *
 * @author w00493811
 * @since 2020-12-28
 */
public interface InitializeStoragePool {
    /**
     * 执行动作:
     * 设置存储池参数
     *
     * @param deviceId deviceId
     * @param username username
     * @param storagePool 存储池
     * @param result 动作结果
     * @return 动作结果
     */
    InitBackActionResult doAction(String deviceId, String username,
        StoragePool storagePool, InitBackActionResult result);
}