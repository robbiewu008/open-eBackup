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

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;

import lombok.AllArgsConstructor;

import org.springframework.stereotype.Component;

/**
 * 端口工厂
 *
 */
@Component
@AllArgsConstructor
public class PortFactory {
    private final EthPort ethPort;
    private final BondPort bondPort;
    private final VlanPort vlanPort;

    /**
     * 工厂函数
     *
     * @param homePortType 主端口类型
     * @return 物理口实例
     */
    public BasePort createPort(HomePortType homePortType) {
        switch (homePortType) {
            case ETHERNETPORT:
                return ethPort;
            case BINDING:
                return bondPort;
            case VLAN:
                return vlanPort;
            default:
                return new BasePort();
        }
    }
}
