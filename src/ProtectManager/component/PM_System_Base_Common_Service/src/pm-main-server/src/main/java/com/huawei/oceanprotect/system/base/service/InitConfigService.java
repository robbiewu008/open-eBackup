/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service;

import openbackup.system.base.sdk.system.model.StorageAuth;

/**
 * 获取配置信息service类
 *
 * @author swx1010572
 * @version: [DataBackup 1.5.0]
 * @since 2023-07-25
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
