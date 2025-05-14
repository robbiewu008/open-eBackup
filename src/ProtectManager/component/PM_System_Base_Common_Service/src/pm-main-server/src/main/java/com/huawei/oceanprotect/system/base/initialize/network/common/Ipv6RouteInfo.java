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
import openbackup.system.base.common.utils.network.AddressUtil;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Pattern;

/**
 * 路由配置信息
 *
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class Ipv6RouteInfo {
    /**
     * 目的地址
     */
    @Pattern(regexp = AddressUtil.IPV6REG, message = "please input correct ipv6 targetAddress")
    private String targetAddress;

    /**
     * 前缀长度
     */
    @Length(min = 1, max = 3, message = "The length of description is 1 ~ 3")
    private String prefixLength;

    /**
     * 网关
     */
    @Pattern(regexp = AddressUtil.IPV6REG, message = "please input correct ipv6 gateway")
    private String gateway;

    /**
     * 无参构造
     */
    public Ipv6RouteInfo() {
    }

    public Ipv6RouteInfo(@Pattern(regexp = AddressUtil.IPV6REG,
        message = "please input correct ipv6 targetAddress") String targetAddress,
        @Length(min = 1, max = 3, message = "The length of description is 1 ~ 3") String prefixLength,
        @Pattern(regexp = AddressUtil.IPV6REG, message = "please input correct ipv6 gateway") String gateway) {
        this.targetAddress = targetAddress;
        this.prefixLength = prefixLength;
        this.gateway = gateway;
    }

    /**
     * 比较Ipv6路由内容是否完全一致
     *
     * @param route 比较资源
     * @return 是否内容相等
     */
    public boolean compare(Ipv6RouteInfo route) {
        if (route == null) {
            return false;
        }
        if (this == route) {
            return true;
        }
        if ((!route.getTargetAddress().equals(targetAddress)) || (!route.getPrefixLength().equals(prefixLength))
            || (!route.getGateway().equals(gateway))) {
            return false;
        }
        return true;
    }

    /**
     * 获取构造新的ipv6PlaneRoute
     *
     * @param ipv6PlaneRoute ipv6PlaneRoute
     * @return 新的ipv6PlaneRoute
     */
    public static Ipv6RouteInfo castFromIpv6RouteInfo(NetPlaneRoute ipv6PlaneRoute) {
        Ipv6RouteInfo ipv6RouteInfo = new Ipv6RouteInfo();
        ipv6RouteInfo.setTargetAddress(ipv6PlaneRoute.getDestination());
        ipv6RouteInfo.setPrefixLength(ipv6PlaneRoute.getMask());
        ipv6RouteInfo.setGateway(ipv6PlaneRoute.getGateWay());
        return ipv6RouteInfo;
    }

    /**
     * 获取构造新的InitRouteInfo
     *
     * @return 新的initRouteInfo
     */
    public InitRouteInfo castFromInitRouteInfo() {
        return new InitRouteInfo(targetAddress, prefixLength, gateway);
    }
}