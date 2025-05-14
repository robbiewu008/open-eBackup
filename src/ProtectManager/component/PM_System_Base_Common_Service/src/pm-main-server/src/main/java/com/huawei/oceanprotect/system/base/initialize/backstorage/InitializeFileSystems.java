/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.backstorage;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

/**
 * 初始化动作
 *
 * @author w00493811
 * @since 2020-12-21
 */
public interface InitializeFileSystems {
    /**
     * 创建文件系统类型
     *
     * @param service 本地存储Service
     * @return 文件系统类型ID
     */
    long attainSetWorkloadTypeId(DeviceManagerService service);
}