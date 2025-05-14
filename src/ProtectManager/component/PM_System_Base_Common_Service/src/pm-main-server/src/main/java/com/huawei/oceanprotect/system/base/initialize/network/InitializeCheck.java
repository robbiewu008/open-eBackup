/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 初始化 检测动作
 *
 * @author swx1010572
 * @since 2021-04-01
 */
public interface InitializeCheck {
    /**
     * 初始化 过程检测检查
     *
     * @param service 设备管理服务
     * @param initNetworkBody 网络配置参数检测
     * @throws LegoCheckedException 检查异常
     */
    void initCheck(DeviceManagerService service, InitNetworkBody initNetworkBody) throws LegoCheckedException;
}
