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
