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

import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.license.common.constans.LicenseInfoConstants;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;
import com.huawei.oceanprotect.system.base.service.DoradoService;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.system.model.StorageAuth;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

/**
 * dorado service impl
 *
 * @author g00500588
 * @since 2021-01-20
 */
@Service
@Slf4j
public class DoradoServiceImpl implements DoradoService {
    @Autowired
    private ClusterInternalApi clusterInternalApi;

    @Autowired
    private IStorageDeviceRepository repository;

    @Autowired
    private DeviceManagerHandler deviceManagerHandler;

    /**
     * 插叙设备esn
     *
     * @return esn
     */
    @Override
    public String queryDevEsn() {
        ClusterDetailInfo clusterDetailInfo = clusterInternalApi.queryClusterDetails();
        if (clusterDetailInfo == null || clusterDetailInfo.getStorageSystem() == null) {
            return LicenseInfoConstants.EMPTY_STRING;
        }
        return clusterDetailInfo.getStorageSystem().getStorageEsn();
    }

    /**
     * 从数据库中获取用户名和密码连接dorado
     *
     * @return DeviceManagerService连接对象
     */
    @Override
    public DeviceManagerService queryDeviceManager() {
        StorageDevice storageDevice = repository.findLocalStorage(true);
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername(storageDevice.getUserName());
        storageAuth.setPassword(storageDevice.getPassword());
        return deviceManagerHandler.achiveDeviceManagerService(storageAuth);
    }
}
