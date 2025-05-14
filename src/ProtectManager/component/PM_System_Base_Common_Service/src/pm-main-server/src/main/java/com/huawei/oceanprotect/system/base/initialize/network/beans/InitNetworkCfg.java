/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.model.NetworkInfo;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * 功能描述
 *
 * @author z00842230
 * @since 2024-03-05
 */
@AllArgsConstructor
@NoArgsConstructor
@Getter
@Setter
public class InitNetworkCfg {
    /**
     * 业务端口类型
     */
    private String servicePortType;

    /**
     * 控制器
     */
    private String controller;

    /**
     * 以太网端口
     */
    private String ethernetPort;

    /**
     * 端口类型
     */
    private String portType;

    /**
     * vlan ID
     */
    private String vlanID;

    /**
     * 网络信息
     */
    private NetworkInfo networkInfo;

    /**
     * 路由
     */
    private List<PortRouteInfo> route;

    /**
     * 创建绑定端口名称
     */
    private String bondPortName;
}