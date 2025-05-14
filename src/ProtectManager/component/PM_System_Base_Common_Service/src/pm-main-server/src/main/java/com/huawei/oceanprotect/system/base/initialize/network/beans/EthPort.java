/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 *
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.BaseObject;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.FailovergroupMemberType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.SystemService;
import com.huawei.oceanprotect.system.base.vo.DeviceInfo;

import lombok.AllArgsConstructor;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 以太网端口
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/25
 */
@AllArgsConstructor
@Component
public class EthPort extends BasePort {
    private final NetWorkPortService netWorkPortService;
    private final SystemService systemService;

    @Override
    public String queryHomePortId(String condition) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        List<com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort> ethPortList =
                netWorkPortService.queryEthPorts(deviceInfo.getEsn(), deviceInfo.getUsername()).getData();
                return ethPortList.stream()
                        .filter(ethPort -> ethPort.getLocation()
                                .equals(condition))
                        .map(BaseObject::getId).findFirst()
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                                "Query eth port id failed, eth port not exist."));
    }

    @Override
    public String queryHomePortName(String portId) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        List<com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort> ethPortList =
                netWorkPortService.queryEthPorts(deviceInfo.getEsn(), deviceInfo.getUsername()).getData();
                return ethPortList.stream()
                        .filter(ethPort -> ethPort.getId()
                                .equals(portId))
                        .map(BaseObject::getLocation).findFirst()
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                                "Query eth port id failed, eth port not exist."));
    }

    @Override
    public FailovergroupMemberType convertToFailOverGroupMemberType() {
        return FailovergroupMemberType.ETH;
    }
}
