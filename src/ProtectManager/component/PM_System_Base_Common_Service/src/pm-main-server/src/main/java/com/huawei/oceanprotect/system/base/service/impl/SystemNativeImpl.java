/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 *
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
 * @author n30046257
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
