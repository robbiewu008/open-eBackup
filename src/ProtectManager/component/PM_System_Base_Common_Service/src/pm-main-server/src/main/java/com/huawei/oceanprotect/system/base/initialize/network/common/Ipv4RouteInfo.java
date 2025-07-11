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
package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitRouteInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplaneroute.NetPlaneRoute;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import javax.validation.constraints.Pattern;

/**
 * 路由配置信息
 *
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class Ipv4RouteInfo {
    /**
     * 目的地址
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct ipv4 targetAddress")
    private String targetAddress;

    /**
     * 子网掩码
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct ipv4 subNetMask")
    private String subNetMask;

    /**
     * 网关
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct ipv4 gateway")
    private String gateway;

    /**
     * 无参构造
     */
    public Ipv4RouteInfo() {
    }

    public Ipv4RouteInfo(@Pattern(regexp = RegexpConstants.IPADDRESS_V4,
        message = "please input correct ipv4 targetAddress") String targetAddress,
        @Pattern(regexp = RegexpConstants.IPADDRESS_V4,
            message = "please input correct ipv4 subNetMask") String subNetMask,
        @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct ipv4 gateway") String gateway) {
        this.targetAddress = targetAddress;
        this.subNetMask = subNetMask;
        this.gateway = gateway;
    }

    /**
     * 比较Ipv4路由内容是否完全一致
     *
     * @param route 比较资源
     * @return 是否内容相等
     */
    public boolean compare(Ipv4RouteInfo route) {
        if (route == null) {
            return false;
        }
        if (this == route) {
            return true;
        }
        if ((!route.getTargetAddress().equals(targetAddress)) || (!route.getSubNetMask().equals(subNetMask))
            || (!route.getGateway().equals(gateway))) {
            return false;
        }
        return true;
    }

    /**
     * 获取构造新的Ipv4RouteInfo
     *
     * @param ipv4PlaneRoute Ipv4PlaneRoute
     * @return 新的Ipv4RouteInfo
     */
    public static Ipv4RouteInfo castFromIpv4RouteInfo(NetPlaneRoute ipv4PlaneRoute) {
        Ipv4RouteInfo ipv4RouteInfo = new Ipv4RouteInfo();
        ipv4RouteInfo.setTargetAddress(ipv4PlaneRoute.getDestination());
        ipv4RouteInfo.setSubNetMask(ipv4PlaneRoute.getMask());
        ipv4RouteInfo.setGateway(ipv4PlaneRoute.getGateWay());
        return ipv4RouteInfo;
    }

    /**
     * 获取构造新的InitRouteInfo
     *
     * @return 新的initRouteInfo
     */
    public InitRouteInfo castFromInitRouteInfo() {
        return new InitRouteInfo(targetAddress, subNetMask, gateway);
    }
}