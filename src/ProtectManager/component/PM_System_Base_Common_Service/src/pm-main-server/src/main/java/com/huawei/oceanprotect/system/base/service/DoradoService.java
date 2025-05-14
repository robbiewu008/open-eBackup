/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

/**
 * dorado service
 *
 * @author g00500588
 * @since 2021-01-11
 */
public interface DoradoService {
    /**
     * Query dev esn string.
     *
     * @return the string
     */
    String queryDevEsn();

    /**
     * 从数据库中获取用户名和密码连接dorado
     *
     * @return DeviceManagerService连接对象
     */
    DeviceManagerService queryDeviceManager();
}
