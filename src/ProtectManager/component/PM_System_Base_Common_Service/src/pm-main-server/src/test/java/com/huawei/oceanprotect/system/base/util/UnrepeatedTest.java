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
package com.huawei.oceanprotect.system.base.util;

import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv6RouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.util.UnrepeatedUtils;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * 路由处理公共测试类
 *
 * @since 2021-04-08
 */
public class UnrepeatedTest {
    /**
     * 测试去重ipv4是否合理
     */
    @Test
    public void test_unrepeatedRoutingIpv4_success() {
        List<Ipv4RouteInfo> routeCfg = new ArrayList<>();
        Ipv4RouteInfo ipv4RouteInfo1 = new Ipv4RouteInfo();
        ipv4RouteInfo1.setTargetAddress("8.40.0.0");
        ipv4RouteInfo1.setSubNetMask("255.255.0.0");
        ipv4RouteInfo1.setGateway("192.168.0.1");
        Ipv4RouteInfo ipv4RouteInfo2 = new Ipv4RouteInfo();
        ipv4RouteInfo2.setTargetAddress("8.40.0.0");
        ipv4RouteInfo2.setSubNetMask("255.255.0.0");
        ipv4RouteInfo2.setGateway("192.168.0.1");
        Ipv4RouteInfo ipv4RouteInfo3 = new Ipv4RouteInfo();
        ipv4RouteInfo3.setTargetAddress("8.80.0.0");
        ipv4RouteInfo3.setSubNetMask("255.255.0.0");
        ipv4RouteInfo3.setGateway("192.168.0.1");
        routeCfg.add(ipv4RouteInfo1);
        routeCfg.add(ipv4RouteInfo2);
        routeCfg.add(ipv4RouteInfo3);
        List<Ipv4RouteInfo> ipv4RouteInfos = UnrepeatedUtils.unrepeatedRoutingIpv4(routeCfg);
        Assert.assertTrue(ipv4RouteInfos.size() == 2);
    }

    /**
     * 测试去重ipv6是否合理
     */
    @Test
    public void test_unrepeatedRoutingIpv6_success() {
        List<Ipv6RouteInfo> routeCfg = new ArrayList<>();
        Ipv6RouteInfo ipv6RouteInfo1 = new Ipv6RouteInfo();
        ipv6RouteInfo1.setTargetAddress("3000:8:42:96:c11::");
        ipv6RouteInfo1.setPrefixLength("122");
        ipv6RouteInfo1.setGateway("2017:8:42:96:c11::1");
        Ipv6RouteInfo ipv6RouteInfo2 = new Ipv6RouteInfo();
        ipv6RouteInfo2.setTargetAddress("3000:8:42:96:c11::");
        ipv6RouteInfo2.setPrefixLength("122");
        ipv6RouteInfo2.setGateway("2017:8:42:96:c11::1");
        Ipv6RouteInfo ipv6RouteInfo3 = new Ipv6RouteInfo();
        ipv6RouteInfo3.setTargetAddress("3000:8:42:96::");
        ipv6RouteInfo3.setPrefixLength("64");
        ipv6RouteInfo3.setGateway("2017:8:42:96:c11::1");
        routeCfg.add(ipv6RouteInfo1);
        routeCfg.add(ipv6RouteInfo2);
        routeCfg.add(ipv6RouteInfo3);
        List<Ipv6RouteInfo> ipv6RouteInfos = UnrepeatedUtils.unrepeatedRoutingIpv6(routeCfg);
        Assert.assertTrue(ipv6RouteInfos.size() == 2);
    }

    /**
     * 测试去重Dns是否合理
     */
    @Test
    public void test_unrepeatedDns_success() {
        ArrayList<String> DnsServer = new ArrayList<>();
        DnsServer.add("2016:8:40:96:c11::90");
        DnsServer.add("2016:8:40:96:c11::91");
        DnsServer.add("2016:8:40:96:c11::91");
        List<String> unrepeatedDns = UnrepeatedUtils.unrepeatedDns(DnsServer);
        System.out.println(unrepeatedDns);
        Assert.assertTrue(unrepeatedDns.size() == 2);
    }

    /**
     * 测试dns为空情况是否合理
     */
    @Test
    public void test_DnsIsNull_success() {
        ArrayList<String> DnsServer = new ArrayList<>();
        DnsServer.add("");
        DnsServer.add("");
        List<String> unrepeatedDns = UnrepeatedUtils.unrepeatedDns(DnsServer);
        System.out.println(unrepeatedDns);
        Assert.assertTrue(unrepeatedDns.size() == 0);
    }
}
