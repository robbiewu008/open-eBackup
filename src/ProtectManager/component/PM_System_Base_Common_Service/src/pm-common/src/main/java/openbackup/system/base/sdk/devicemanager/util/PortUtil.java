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
package openbackup.system.base.sdk.devicemanager.util;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.network.IPv6Address;
import openbackup.system.base.common.utils.network.IPv6NetworkMask;
import openbackup.system.base.sdk.cluster.netplane.PortRoute;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;
import sun.net.util.IPAddressUtil;

import org.springframework.beans.BeanUtils;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.util.TreeSet;

/**
 * PortUtil
 *
 */
@Slf4j
public class PortUtil {
    /**
     * 将接口的portRoute转化为底座的DTO
     *
     * @param portRoute portRoute
     * @return portRouteInfo
     */
    public static PortRouteInfo convertPortRoute(PortRoute portRoute) {
        PortRouteInfo portRouteInfo = new PortRouteInfo();
        BeanUtils.copyProperties(portRoute, portRouteInfo);
        portRouteInfo.setRouteType(RouteType.forValues(portRoute.getRouteType()));
        return portRouteInfo;
    }

    /**
     * 检测是不是相同的路由
     *
     * @param routeA routeA
     * @param routeB routeB
     * @return 是否是相同的路由
     */
    public static boolean isSameRoute(PortRouteInfo routeA, PortRouteInfo routeB) {
        return isRouteDestinationSame(routeA, routeB) && Objects.equals(routeA.getGateway(), routeB.getGateway());
    }

    /**
     * 路由目标IP是否相同
     *
     * @param routeA routeA
     * @param routeB routeB
     * @return 路由目标IP是否相同
     */
    public static boolean isRouteDestinationSame(PortRouteInfo routeA, PortRouteInfo routeB) {
        return Objects.equals(routeA.getMask(), routeB.getMask()) && Objects.equals(routeA.getDestination(),
            routeB.getDestination());
    }

    /**
     * 判断路由列表是否冲突
     *
     * @param portRoutes 路由信息
     */
    public static void checkPortRouteConflict(List<PortRoute> portRoutes) {
        TreeSet<PortRoute> compareSet = new TreeSet<>(
            (r1, r2) -> isSameRoute(convertPortRoute(r1), convertPortRoute(r2)) ? 0 : 1);
        compareSet.addAll(portRoutes);
        if (compareSet.size() != portRoutes.size()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Port route conflict");
        }
    }

    /**
     * 检测两个ip（V4或V6）的网段是否相同
     *
     * @param ip1  ip1
     * @param mask1 掩码1
     * @param ip2 ip2
     * @param mask2 掩码2
     * @return 是否相同
     */
    public static boolean isSubnetSame(String ip1, String mask1, String ip2, String mask2) {
        if (IPAddressUtil.isIPv4LiteralAddress(ip1) && IPAddressUtil.isIPv4LiteralAddress(ip2)) {
            return isIPV4SubnetSame(ip1, mask1, ip2, mask2);
        }

        if (IPAddressUtil.isIPv6LiteralAddress(ip1) && IPAddressUtil.isIPv6LiteralAddress(ip2)) {
            return isIPV6SubnetSame(ip1, mask1, ip2, mask2);
        }
        return false;
    }

    private static boolean isIPV4SubnetSame(String ip1, String mask1, String ip2, String mask2) {
        return Arrays.equals(getSubnetIpv4(ip1, mask1), getSubnetIpv4(ip2, mask2));
    }

    private static byte[] getSubnetIpv4(String ip, String mask) {
        byte[] formatV4 = IPAddressUtil.textToNumericFormatV4(ip);
        byte[] formatV4mask = IPAddressUtil.textToNumericFormatV4(mask);
        for (int i = 0; i < formatV4.length; i++) {
            formatV4[i] &= formatV4mask[i];
        }
        return formatV4;
    }

    private static boolean isIPV6SubnetSame(String ip1, String mask1, String ip2, String mask2) {
        if (!Objects.equals(mask1, mask2)) {
            return false;
        }
        IPv6Address subnetAddress1 = IPv6Address.fromString(ip1)
            .maskWithNetworkMask(IPv6NetworkMask.fromPrefixLength(Integer.parseInt(mask1)));
        IPv6Address subnetAddress2 = IPv6Address.fromString(ip2)
            .maskWithNetworkMask(IPv6NetworkMask.fromPrefixLength(Integer.parseInt(mask2)));
        return subnetAddress1.compareTo(subnetAddress2) == 0;
    }
}
