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
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.vlan.VlanInfo;
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
 * vlan
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/25
 */
@AllArgsConstructor
@Component
public class VlanPort extends BasePort {
    private final NetWorkPortService netWorkPortService;
    private final SystemService systemService;

    @Override
    public String queryHomePortId(String condition) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        List<VlanInfo> vlanPortList =
                netWorkPortService.queryVlan(deviceInfo.getEsn(), deviceInfo.getUsername()).getData();
                return vlanPortList.stream()
                        .filter(vlanInfo -> vlanInfo.getName()
                                .equals(condition))
                        .map(BaseObject::getId).findFirst()
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                                "Query vlan port id failed, vlan port not exist."));
    }

    @Override
    public String queryHomePortName(String portId) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        List<VlanInfo> vlanPortList = netWorkPortService.queryVlan(deviceInfo.getEsn(),
                deviceInfo.getUsername()).getData();
                return vlanPortList.stream()
                        .filter(vlanInfo -> vlanInfo.getId()
                                .equals(portId))
                        .map(BaseObject::getName).findFirst()
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                                "Query vlan port id failed, vlan port not exist."));
    }

    @Override
    public FailovergroupMemberType convertToFailOverGroupMemberType() {
        return FailovergroupMemberType.VLAN;
    }
}
