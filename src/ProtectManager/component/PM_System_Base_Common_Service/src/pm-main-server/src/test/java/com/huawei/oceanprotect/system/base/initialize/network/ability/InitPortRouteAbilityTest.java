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
package com.huawei.oceanprotect.system.base.initialize.network.ability;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkResult;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplaneroute.NetPlaneRoute;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.List;

/**
 * InitPortRouteAbility Test
 *
 * @since 2023-02-27
 */
public class InitPortRouteAbilityTest {

    @Test
    public void doAction_success() {
        // MOCK 默认认为成功
        DeviceManagerService service = mockSuccessService("parentId");

        // test 开始
        List<Ipv4RouteInfo> ipv4RouteCfg = Lists.newArrayList(createIpv4RouteInfo());
        List<String> portHomeIds = Lists.newArrayList("22");
        InitPortRouteAbility ability = new InitPortRouteAbility();
        InitNetworkResult result = ability.doAction(service, ipv4RouteCfg, portHomeIds);
        Assert.assertTrue(result.isOkay());
    }

    @Test
    public void delNetPlaneRoute_success() {
        String parentId = "parentId";

        // MOCK 默认认为成功
        DeviceManagerService service = mockSuccessService("parentId");

        // test 开始
        List<Ipv4RouteInfo> ipv4RouteCfg = Lists.newArrayList(createIpv4RouteInfo());
        InitPortRouteAbility ability = new InitPortRouteAbility();
        InitNetworkResult result = ability.delNetPlaneRoute(service, parentId, ipv4RouteCfg);
        Assert.assertTrue(result.isOkay());
    }

    @Test
    public void addNetPlaneRoute() {
        // MOCK 默认认为成功
        DeviceManagerService service = mockSuccessService("parentId");

        // test 开始
        List<Ipv4RouteInfo> ipv4RouteCfg = Lists.newArrayList(createIpv4RouteInfo());
        InitPortRouteAbility ability = new InitPortRouteAbility();
        ability.addNetPlaneRoute(service, "backupType",ipv4RouteCfg);
    }

    @Test
    public void getNetPlaneRoute() {
        // MOCK 默认认为成功
        DeviceManagerService service = mockSuccessService("parentId");

        // test 开始
        List<Ipv4RouteInfo> ipv4RouteCfg = Lists.newArrayList(createIpv4RouteInfo());
        InitPortRouteAbility ability = new InitPortRouteAbility();
        ability.addNetPlaneRoute(service, "backupType",ipv4RouteCfg);
    }

    private static DeviceManagerService mockSuccessService(String parentId) {
        // MOCK 默认认为成功
        DeviceManagerService service = PowerMockito.mock(DeviceManagerService.class);
        DeviceManagerResponseError error = PowerMockito.mock(DeviceManagerResponseError.class);
        PowerMockito.when(error.isSuccess()).thenReturn(true);
        DeviceManagerResponse<PortRouteInfo> route = PowerMockito.mock(DeviceManagerResponse.class);
        PowerMockito.when(route.getError()).thenReturn(error);

        // mock 网络平面路由查询
        DeviceManagerResponse<List<NetPlaneRoute>> netPlaneRoutes = PowerMockito.mock(DeviceManagerResponse.class);
        PowerMockito.when(netPlaneRoutes.getError()).thenReturn(error);
        NetPlaneRoute planeRoute = new NetPlaneRoute();
        planeRoute.setParentID(parentId);
        planeRoute.setGateWay("192.168.10.1");
        planeRoute.setMask("255.255.0.0");
        planeRoute.setDestination("destination");
        List<NetPlaneRoute> planeRoutes = Lists.newArrayList(planeRoute);
        PowerMockito.when(netPlaneRoutes.getData()).thenReturn(planeRoutes);
        PowerMockito.when(service.getNetPlaneRoute()).thenReturn(netPlaneRoutes);

        // mock 路由信息查询
        DeviceManagerResponse<List<PortRouteInfo>> portRoutes = PowerMockito.mock(DeviceManagerResponse.class);
        PowerMockito.when(portRoutes.getError()).thenReturn(error);
        List<PortRouteInfo> routeInfos = Lists.newArrayList(new PortRouteInfo());
        PowerMockito.when(portRoutes.getData()).thenReturn(routeInfos);
        PowerMockito.when(service.getPortRoutes(anyString())).thenReturn(portRoutes);


        // mock 路由信息删除
        PowerMockito.when(service.deletePortRoutes(any())).thenReturn(route);

        // mock 路由信息添加
        PowerMockito.when(service.addPortRoutes(any())).thenReturn(route);
        return service;
    }


    private List<Ipv4RouteInfo> createIpv4RouteInfo() {
        List<Ipv4RouteInfo> ipv4 = new ArrayList<>();
        Ipv4RouteInfo ipv4RouteInfo = new Ipv4RouteInfo();
        ipv4RouteInfo.setTargetAddress("51.6.135.0");
        ipv4RouteInfo.setSubNetMask("255.255.255.0");
        ipv4RouteInfo.setGateway("51.6.135.1");
        ipv4.add(ipv4RouteInfo);
        ipv4RouteInfo.setTargetAddress("8.40.0.0");
        ipv4RouteInfo.setSubNetMask("255.255.0.0");
        ipv4RouteInfo.setGateway("192.168.10.1");
        ipv4.add(ipv4RouteInfo);
        return ipv4;
    }
}