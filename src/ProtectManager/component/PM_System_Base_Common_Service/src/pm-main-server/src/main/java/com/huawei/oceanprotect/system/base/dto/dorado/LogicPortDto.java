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
package com.huawei.oceanprotect.system.base.dto.dorado;

import com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.model.BondPortPo;
import com.huawei.oceanprotect.system.base.model.VlanPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.SupportProtocol;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.common.enums.AddressFamily;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import org.apache.commons.lang3.StringUtils;

import java.util.List;

/**
 * 对外体现逻辑端口列表
 *
 */
@Data
public class LogicPortDto {
    /**
     * id
     */
    private String id;

    /**
     * 名称
     */
    private String name;

    /**
     * 逻辑端口归属名称
     */
    private String homePortName;

    /**
     * 端口逻辑类型
     */
    private HomePortType homePortType;

    /**
     * 父端口ID
     */
    private String homePortId;

    /**
     * ip
     */
    private String ip;

    /**
     * 子网掩码
     */
    private String mask;

    /**
     * 网关
     */
    private String gateWay;

    /**
     * 逻辑端口角色
     */
    private PortRole role;

    /**
     * IP类型
     */
    private String ipType;

    /**
     * 协议
     */
    private SupportProtocol supportProtocol;

    /**
     * 父端口控制器ID
     */
    private String homeControllerId;

    /**
     * 当前端口控制器ID
     */
    private String currentControllerId;

    /**
     * 业务端口配置的Vlan信息
     */
    private VlanPo vlan;

    /**
     * 业务端口配置的绑定端口
     */
    private BondPortPo bondPort;

    /**
     * 业务端口配置的路由
     */
    private List<PortRouteInfo> route;

    /**
     * 漂移组ID
     */
    private String failoverGroupId;

    /**
     * 业务端口在DM是否存在
     */
    private boolean isDmExists;

    /**
     * 业务端口是否生效
     */
    @JsonProperty("isValid")
    private boolean isValid;

    /**
     * 底座真正创建的角色类型
     */
    @JsonIgnore
    private PortRole dmRole;

    /**
     * 是否启用漂移
     */
    private Boolean isFailOver;

    /**
     * 构造NFSCIFS逻辑端口
     *
     * @return NFSCIFS逻辑端口
     */
    public LogicPortAddRequest copyToBackupLogicPort() {
        LogicPortAddRequest logicPortAddRequest = new LogicPortAddRequest();
        logicPortAddRequest.setSupportProtocol(SupportProtocol.NFS_CIFS);
        logicPortAddRequest.setVstoreId(InitConfigConstant.VSTORE_ID);
        copyToBaseLogicPort(logicPortAddRequest);
        return logicPortAddRequest;
    }

    /**
     * 构造DataTurbo逻辑端口
     *
     * @return DataTurbo逻辑端口
     */
    public LogicPortAddRequest copyToDataTurboLogicPort() {
        LogicPortAddRequest logicPortAddRequest = new LogicPortAddRequest();
        logicPortAddRequest.setSupportProtocol(supportProtocol);
        copyToBaseLogicPort(logicPortAddRequest);
        return logicPortAddRequest;
    }

    /**
     * 构造复制逻辑端口
     *
     * @return 复制逻辑端口
     */
    public LogicPortAddRequest copyToDuplicateLogicPort() {
        LogicPortAddRequest logicPortAddRequest = new LogicPortAddRequest();
        copyToBaseLogicPort(logicPortAddRequest);
        return logicPortAddRequest;
    }

    /**
     * 构造归档逻辑端口
     *
     * @return 归档逻辑端口
     */
    public LogicPortAddRequest copyToArchiveLogicPort() {
        LogicPortAddRequest logicPortAddRequest = new LogicPortAddRequest();
        copyToBaseLogicPort(logicPortAddRequest);
        return logicPortAddRequest;
    }

    private void copyToBaseLogicPort(LogicPortAddRequest logicPortAddRequest) {
        logicPortAddRequest.setHomePortId(homePortId);
        logicPortAddRequest.setHomePortType(homePortType);
        logicPortAddRequest.setName(name);
        logicPortAddRequest.setRole(dmRole);
        logicPortAddRequest.setEnableRouteSrc(InitNetworkConfigConstants.ENABLE_ROUTE_SRC);
        if (PortRole.SERVICE.equals(dmRole)) {
            logicPortAddRequest.setIsFailOver(isFailOver);
            if (Boolean.TRUE.equals(isFailOver) && !StringUtils.isEmpty(failoverGroupId)) {
                logicPortAddRequest.setFailoverGroupId(failoverGroupId);
            }
        }
        if (InitConfigConstant.IPV4_TYPE_FLAG.equals(ipType)) {
            logicPortAddRequest.setAddressFamily(AddressFamily.IPV4);
            logicPortAddRequest.setIpv4Addr(ip);
            logicPortAddRequest.setIpv4Mask(mask);
            logicPortAddRequest.setIpv4Gateway(gateWay);
        } else {
            logicPortAddRequest.setAddressFamily(AddressFamily.IPV6);
            logicPortAddRequest.setIpv6Addr(ip);
            logicPortAddRequest.setIpv6Mask(mask);
            logicPortAddRequest.setIpv6Gateway(gateWay);
        }
    }
}
