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
package com.huawei.oceanprotect.system.base.initialize.backstorage.ability;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeBackStorage;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.storagepool.StoragePool;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.system.SystemInfo;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 初始化备份存储能力
 *
 * @since 2020-12-21
 */
@Slf4j
@Service
public class InitBackStorageAbility implements InitializeBackStorage {
    // 单一文件系统大小
    private static final long FILESYSTEM_SIZE = 10L;

    // 单一文件系统尺寸单位：TB的数值
    private static final long FILESYSTEM_SIZE_UNIT = 1024L * 1024L * 1024L * 1024L;

    // 单一文件系统最小尺寸10TB
    private static final long FILESYSTEM_MIN_SIZE = FILESYSTEM_SIZE * FILESYSTEM_SIZE_UNIT;

    @Autowired
    private InitStatusService initStatusService;

    @Autowired
    private ClusterBasicService clusterBasicService;

    /**
     * 检查
     *
     * @param service 设备管理服务
     * @throws LegoCheckedException 检查异常
     */
    @Override
    public void check(DeviceManagerService service) throws LegoCheckedException {
        // 获取存储池信息
        DeviceManagerResponse<List<StoragePool>> poolResponse = service.getStoragePools();
        List<StoragePool> pools = poolResponse.getData();
        if (CollectionUtils.isEmpty(pools)) {
            // 存储池数量为0，异常(CommonErrorCode.STORAGE_POOL_NUMB_EXCEPTION)
            // 待处理：错误码需待确认和申请
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED);
        }

        // 获取系统信息
        DeviceManagerResponse<SystemInfo> response = service.getSystem();
        SystemInfo systemInfo = response.getData();
        long sectorSize = Long.parseLong(systemInfo.getSectorSize());
        StoragePool pool = pools.get(0);
        long userTotalCapacity = Long.parseLong(pool.getUserTotalCapacity());
        userTotalCapacity = userTotalCapacity * sectorSize;
        if (userTotalCapacity < FILESYSTEM_MIN_SIZE) {
            // 存储池容量太小，无法构造爱数备份存储池
            log.info("InitBackStorageAbility.check: storage size not enough");
            List<String> storageSize = new ArrayList<>();
            storageSize.add(String.valueOf(FILESYSTEM_SIZE) + "TB"); // 容量单位转TB
            initStatusService.setInitProgressParams(storageSize, clusterBasicService.getCurrentClusterEsn());
            throw new LegoCheckedException(CommonErrorCode.STORAGE_POOL_CAPACITY_INSUFFICIENT_EXCEPTION);
        }
    }
}