/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import lombok.Getter;
import lombok.Setter;

/**
 * IP资源的路由配置信息
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-29
 */
@Getter
@Setter
public class InitRouteInfo {
    /**
     * 目的地址
     */
    private String targetAddress;

    /**
     * 子网掩码
     */
    private String subNetMask;

    /**
     * 网关
     */
    private String gateway;

    public InitRouteInfo(String targetAddress, String subNetMask, String gateway) {
        this.targetAddress = targetAddress;
        this.subNetMask = subNetMask;
        this.gateway = gateway;
    }
}