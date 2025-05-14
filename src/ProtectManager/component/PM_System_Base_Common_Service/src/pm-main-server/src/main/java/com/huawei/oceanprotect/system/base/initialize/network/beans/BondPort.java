/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 *
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.BaseObject;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPortRes;
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
 * 绑定端口
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/25
 */
@AllArgsConstructor
@Component
public class BondPort extends BasePort {
    private final NetWorkPortService netWorkPortService;
    private final SystemService systemService;

    @Override
    public String queryHomePortId(String condition) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        List<BondPortRes> bondPort = netWorkPortService.getBondPort(deviceInfo.getEsn(),
                deviceInfo.getUsername()).getData();
                return bondPort.stream()
                        .filter(bondPortRes -> bondPortRes.getName()
                                .equals(condition))
                        .map(BaseObject::getId).findFirst()
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                                "Query bond port id failed, bond port not exist."));
    }

    @Override
    public String queryHomePortName(String portId) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        List<BondPortRes> bondPort = netWorkPortService.getBondPort(deviceInfo.getEsn(),
                deviceInfo.getUsername()).getData();
                return bondPort.stream()
                        .filter(bondPortRes -> bondPortRes.getId()
                                .equals(portId))
                        .map(BaseObject::getName).findFirst()
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                                "Query bond port id failed, bond port not exist."));
    }

    @Override
    public FailovergroupMemberType convertToFailOverGroupMemberType() {
        return FailovergroupMemberType.BOND;
    }
}
