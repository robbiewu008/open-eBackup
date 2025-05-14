/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

/**
 * 集群存储性能
 *
 * @author swx1010572
 * @since 2022-08-18
 */
public interface InitializeSwitchPerformance {
    /**
     * 更新设置性能开关状态
     *
     * @param service dm 对象
     * @param performanceSwitch 设置开启关闭
     */
    void updatePerformanceConfig(DeviceManagerService service, String performanceSwitch);
}
