/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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