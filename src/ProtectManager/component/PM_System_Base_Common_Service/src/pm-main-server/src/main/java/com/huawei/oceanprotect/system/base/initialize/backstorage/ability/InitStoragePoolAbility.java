/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.backstorage.ability;

import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeStoragePool;
import com.huawei.oceanprotect.system.base.initialize.backstorage.beans.InitBackActionResult;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.diskpool.DiskPool;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.storagepool.Disks;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.storagepool.StoragePoolExpandInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.storagepool.Task;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.DiskPoolServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.StoragePoolRestApi;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.model.repository.StoragePool;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

/**
 * 初始化备份存储
 *
 * @author w00493811
 * @since 2020-12-21
 */
@Slf4j
@Service
public class InitStoragePoolAbility implements InitializeStoragePool {
    @Autowired
    private DiskPoolServiceApi diskPoolServiceApi;

    @Autowired
    private StoragePoolRestApi storagePoolRestApi;

    /**
     * 执行动作
     *
     * @param deviceId deviceId
     * @param username username
     * @param storagePool 存储池
     * @param initBackActionResult 动作结果
     * @return 动作结果
     */
    @Override
    public InitBackActionResult doAction(String deviceId, String username, StoragePool storagePool,
        InitBackActionResult initBackActionResult) {
        // 暂时忽略设置硬盘域和磁盘数，可能存在问题
        return doActionOnStoragePool(deviceId, username, storagePool, initBackActionResult);
    }

    /**
     * 暂时忽略设置硬盘域和磁盘数，可能存在问题
     *
     * @param deviceId deviceId
     * @param username username
     * @param storagePool 存储池
     * @param initBackActionResult 结果
     * @return 结果
     */
    private InitBackActionResult doActionOnStoragePool(String deviceId, String username, StoragePool storagePool,
        InitBackActionResult initBackActionResult) {
        log.info("Enter InitStoragePoolAbility.doAction");

        // 1.设置硬盘域，设置其父节点的硬盘域“将SSDDISKNUM设置为-1”
        DiskPool diskPool = new DiskPool();
        diskPool.setId(storagePool.getParentId());
        diskPool.setSsdDiskNum(DiskPool.SSD_DISK_NUM_DEFAULT);
        try {
            DeviceManagerResponse<Object> setDiskPoolInfoResponse = diskPoolServiceApi.setDiskPoolInfo(deviceId,
                username, diskPool.getId(), diskPool);
        } catch (DeviceManagerException exception) {
            log.error("Setup disk pool may be failed: ", exception);
        }

        // 2.设置存储池，将disks字段设置all=true
        StoragePoolExpandInfo storagePoolExpandInfo = new StoragePoolExpandInfo();
        storagePoolExpandInfo.setDisks(new Disks(true));

        try {
            DeviceManagerResponse<Task> setStoragePoolExpandInfoResponse =
                storagePoolRestApi.setStoragePoolExpandInfo(deviceId, username,
                    storagePool.getId(), storagePoolExpandInfo);
        } catch (DeviceManagerException exception) {
            log.error("Setup storage pool expand may be failed: ", exception);
        }
        return new InitBackActionResult();
    }
}