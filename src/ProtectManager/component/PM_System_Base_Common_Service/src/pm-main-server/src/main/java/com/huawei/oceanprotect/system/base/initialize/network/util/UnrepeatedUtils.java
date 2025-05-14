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
package com.huawei.oceanprotect.system.base.initialize.network.util;

import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv6RouteInfo;

import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;
import java.util.stream.Collectors;

/**
 * 路由处理公共类
 *
 * @since 2021-04-08
 */
public class UnrepeatedUtils {
    private UnrepeatedUtils() {
    }

    /**
     * ipv4路由集合去重
     *
     * @param routeCfg 是否所有时间参数
     * @return 时间
     */
    public static List<Ipv4RouteInfo> unrepeatedRoutingIpv4(List<Ipv4RouteInfo> routeCfg) {
        // 判断传入的routeCfg 是否存在
        if (routeCfg.size() == 0) {
            return routeCfg;
        }

        Set<Ipv4RouteInfo> ipv4RouteInfos = new TreeSet<>(
            Comparator.comparing(ipv4 -> ipv4.getTargetAddress() + "" + ipv4.getSubNetMask() + "" + ipv4.getGateway()));
        for (Ipv4RouteInfo ipv4RouteInfo : routeCfg) {
            ipv4RouteInfos.add(ipv4RouteInfo);
        }
        return new ArrayList<>(ipv4RouteInfos);
    }

    /**
     * ipv6路由集合去重
     *
     * @param routeCfg 是否所有时间参数
     * @return 时间
     */
    public static List<Ipv6RouteInfo> unrepeatedRoutingIpv6(List<Ipv6RouteInfo> routeCfg) {
        // 判断传入的routeCfg 是否存在
        if (routeCfg.size() == 0) {
            return routeCfg;
        }

        Set<Ipv6RouteInfo> ipv6RouteInfos = new TreeSet<>(Comparator.comparing(
            ipv6 -> ipv6.getTargetAddress() + "" + ipv6.getPrefixLength() + "" + ipv6.getGateway()));
        for (Ipv6RouteInfo ipv6RouteInfo : routeCfg) {
            ipv6RouteInfos.add(ipv6RouteInfo);
        }
        return new ArrayList<>(ipv6RouteInfos);
    }

    /**
     * Dns参数集合去重
     *
     * @param dnsServer 所有的路由参数
     * @return 时间
     */
    public static List<String> unrepeatedDns(List<String> dnsServer) {
        // 判断传入的dnsServer 是否存在
        ArrayList<String> dns = new ArrayList<>();
        for (String server : dnsServer) {
            if (StringUtils.isEmpty(server)) {
                continue;
            }
            dns.add(server);
        }
        if (dns.size() == 0) {
            return dns;
        }
        return dns.stream().distinct().collect(Collectors.toList());
    }

    /**
     * 获取控制框parentId 集合
     *
     * @param controllerSize 需要的控制个数
     * @return 控制框parentId 集合
     */
    public static List<String> getControllerSize(int controllerSize) {
        int size = controllerSize / 2;
        List<String> podSize = new ArrayList<>();
        for (int i = 0; i < size; i++) {
            podSize.add(String.valueOf(i));
        }
        return podSize;
    }
}
