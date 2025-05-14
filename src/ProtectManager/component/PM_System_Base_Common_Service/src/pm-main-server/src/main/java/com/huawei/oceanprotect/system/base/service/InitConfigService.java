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
package com.huawei.oceanprotect.system.base.service;

import openbackup.system.base.sdk.system.model.StorageAuth;

/**
 * 获取配置信息service类
 *
 */
public interface InitConfigService {
    /**
     * 更新数据库的存储信息
     *
     * @param storageAuth 存储信息
     */
    void updateLocalStorageAuth(StorageAuth storageAuth);

    /**
     * 获取数据库中存储信息
     *
     * @return 数据库中存储信息
     */
    StorageAuth getLocalStorageAuth();

    /**
     * 更新数据库的存储的deviceId
     *
     * @param deviceId 存储信息
     */
    void updateLocalStorageDeviceId(String deviceId);

    /**
     * 获取数据库中存储的deviceId
     *
     * @return 数据库中存储的deviceId
     */
    String getLocalStorageDeviceId();

    /**
     * 更新数据库的存储的deviceIp
     *
     * @param deviceIp 存储信息
     */
    void updateLocalStorageDeviceIp(String deviceIp);

    /**
     * 获取数据库中存储的deviceIp
     *
     * @return 数据库中存储的deviceIp
     */
    String getLocalStorageDeviceIp();
}
