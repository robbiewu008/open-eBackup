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
package com.huawei.oceanprotect.system.base.service.impl;

import com.huawei.oceanprotect.system.base.service.SystemService;

import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.sdk.system.SystemNativeApi;
import openbackup.system.base.sdk.system.model.StorageAuth;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.UUID;

/**
 * 本地系统服务实现
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/3/16
 */
@Service
public class SystemNativeImpl implements SystemNativeApi {
    @Autowired
    private SystemService systemService;

    @Override
    public String checkAuth(StorageAuth authInfo) {
        String randomDeviceId = UUID.randomUUID().toString();
        DeviceUser deviceUser = new DeviceUser();
        deviceUser.setId(randomDeviceId);
        deviceUser.setPassword(authInfo.getPassword());
        deviceUser.setUsername(authInfo.getUsername());
        return systemService.configStorageAuth(deviceUser);
    }
}
