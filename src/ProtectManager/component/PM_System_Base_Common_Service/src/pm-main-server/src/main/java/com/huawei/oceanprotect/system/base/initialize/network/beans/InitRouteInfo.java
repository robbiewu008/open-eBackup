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

import lombok.Getter;
import lombok.Setter;

/**
 * IP资源的路由配置信息
 *
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