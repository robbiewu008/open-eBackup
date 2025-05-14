/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.system.base.initialize.network.enums.NetworkType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

import openbackup.system.base.util.Applicable;

/**
 * 处理网络配置的接口
 *
 * @author swx1010572
 * @since 2021-11-29
 */
public interface GetNetworkConfigService<T> extends Applicable<String> {
    /**
     * 获取网络配置信息
     *
     * @param service DM 对象
     * @param ipType ip类型
     * @param networkType 网络类型
     * @return 网络平面信息对象
     */
    T getNetworkConfig(DeviceManagerService service, String ipType, NetworkType networkType) ;
}
