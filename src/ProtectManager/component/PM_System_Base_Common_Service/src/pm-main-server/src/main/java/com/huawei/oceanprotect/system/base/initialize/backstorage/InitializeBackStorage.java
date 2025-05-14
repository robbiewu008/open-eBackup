/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.backstorage;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 初始化动作
 *
 * @author w00493811
 * @since 2020-12-21
 */
public interface InitializeBackStorage {
    /**
     * 检查
     *
     * @param service 设备管理服务
     * @throws LegoCheckedException 检查异常
     */
    void check(DeviceManagerService service) throws LegoCheckedException;
}