/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 *
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;

import lombok.AllArgsConstructor;

import org.springframework.stereotype.Component;

/**
 * 端口工厂
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/25
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
