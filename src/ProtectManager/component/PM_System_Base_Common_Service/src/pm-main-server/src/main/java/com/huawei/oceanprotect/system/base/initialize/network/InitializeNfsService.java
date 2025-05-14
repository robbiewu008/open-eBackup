/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

/**
 * 添加DM端口
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-27
 */
public interface InitializeNfsService {
    /**
     * 更行NFSService4.1的信息
     *
     * @param service dm 对象
     */
    void modifyNfsService(DeviceManagerService service);
}
