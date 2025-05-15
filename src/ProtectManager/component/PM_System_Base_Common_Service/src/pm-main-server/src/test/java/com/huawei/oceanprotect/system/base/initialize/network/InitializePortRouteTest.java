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
package com.huawei.oceanprotect.system.base.initialize.network;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import com.huawei.oceanprotect.system.base.initialize.network.ability.InitPortRouteAbility;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkResult;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplaneroute.NetPlaneRoute;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * 测试 路由创建情况
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    InitializePortRoute.class, InitPortRouteAbility.class
})
public class InitializePortRouteTest {

    /**
     * 字符串0
     */
    static final String STRING0 = "0";

    @MockBean
    private DeviceManagerService service;

    @MockBean
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private InitializePortRoute initializePortRoute;

    @Before
    public void init() {
        given(service.getPortRoutes(any())).willReturn(getDeviceManagerResponseRouteInfo());
        given(service.deletePortRoutes(any())).willReturn(deviceManagerResponseSuccess(PortRouteInfo.class));
        given(service.addPortRoutes(any())).willReturn(deviceManagerResponseSuccess(PortRouteInfo.class));
        given(service.getNetPlaneRoute()).willReturn(getDeviceManagerResponseNetPlaneRoute());
    }

    /**
     * 用例场景：为逻辑端口添加路由信息
     * 前置条件：NA
     * 检查点：添加成功无报错
     */
    @Test
    public void add_logic_port_route_success() {
        InitNetworkResult status = initializePortRoute.doAction(service, createIpv4RouteInfo(), getLogicPortId());
        Assert.assertTrue(status.isOkay());
    }

    /**
     * 用例场景：为平面网络添加路由信息
     * 前置条件：NA
     * 检查点：添加成功无报错
     */
    @Test
    public void add_net_plane_route_success() {
        Assert.assertTrue(initializePortRoute.addNetPlaneRoute(service, "1", getIpv4RouteInfos()).isOkay());
    }

    /**
     * 用例场景：删除平面网络路由信息
     * 前置条件：NA
     * 检查点：添加成功无报错
     */
    @Test
    public void del_net_plane_route_success() {
        Assert.assertTrue(initializePortRoute.delNetPlaneRoute(service, "1", getIpv4RouteInfos()).isOkay());
    }

    /**
     * 用例场景：获取平面网络路由信息
     * 前置条件：NA
     * 检查点：成功获取返回值
     */
    @Test
    public void get_net_plane_route_success() {
        Assert.assertEquals(1, initializePortRoute.getNetPlaneRoute(service, "1").size());
    }

    private List<Ipv4RouteInfo> getIpv4RouteInfos() {
        List<Ipv4RouteInfo> ipv4RouteInfoList = new ArrayList<>();
        Ipv4RouteInfo ipv4RouteInfo1 = new Ipv4RouteInfo();
        ipv4RouteInfo1.setTargetAddress("192.168.100.10");
        ipv4RouteInfo1.setSubNetMask("255.255.0.0");
        ipv4RouteInfo1.setGateway("192.168.0.1");
        Ipv4RouteInfo ipv4RouteInfo2 = new Ipv4RouteInfo();
        ipv4RouteInfo2.setTargetAddress("192.168.100.11");
        ipv4RouteInfo2.setSubNetMask("255.255.0.0");
        ipv4RouteInfo2.setGateway("192.168.0.1");
        Ipv4RouteInfo ipv4RouteInfo3 = new Ipv4RouteInfo();
        ipv4RouteInfo3.setTargetAddress("8.40.0.0");
        ipv4RouteInfo3.setSubNetMask("255.255.0.0");
        ipv4RouteInfo3.setGateway("192.168.10.1");
        ipv4RouteInfoList.add(ipv4RouteInfo1);
        ipv4RouteInfoList.add(ipv4RouteInfo2);
        ipv4RouteInfoList.add(ipv4RouteInfo3);
        return ipv4RouteInfoList;
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

    /**
     * 模拟返回ethPorts
     *
     * @return DeviceManagerResponse
     */
    private DeviceManagerResponse<List<PortRouteInfo>> getDeviceManagerResponseRouteInfo() {
        PortRouteInfo portRouteInfo = new PortRouteInfo();
        portRouteInfo.setDestination("8.40.0.0");
        portRouteInfo.setMask("255.255.0.0");
        portRouteInfo.setGateway("192.168.10.1");
        List<PortRouteInfo> portRouteInfoList = new ArrayList<>();
        portRouteInfoList.add(portRouteInfo);
        DeviceManagerResponse<List<PortRouteInfo>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        deviceManagerResponse.setData(portRouteInfoList);
        return deviceManagerResponse;
    }

    private DeviceManagerResponse<List<NetPlaneRoute>> getDeviceManagerResponseNetPlaneRoute() {
        NetPlaneRoute netPlaneRoute = new NetPlaneRoute();
        netPlaneRoute.setDestination("8.40.0.0");
        netPlaneRoute.setMask("255.255.0.0");
        netPlaneRoute.setGateWay("192.168.10.1");
        netPlaneRoute.setParentID("1");
        List<NetPlaneRoute> portRouteInfoList = new ArrayList<>();
        portRouteInfoList.add(netPlaneRoute);
        DeviceManagerResponse<List<NetPlaneRoute>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        deviceManagerResponse.setData(portRouteInfoList);
        return deviceManagerResponse;
    }

    /**
     * 模拟返回ethPorts
     *
     * @return DeviceManagerResponse
     */
    private List<String> getLogicPortId() {
        List<String> logicPortId = new ArrayList<>();
        logicPortId.add("5522");
        logicPortId.add("5");
        logicPortId.add("55");
        return logicPortId;
    }

    /**
     * 模拟设置成功
     *
     * @param deviceManagerResponse 将其设为成功
     */
    private void deviceManagerResponseSetErrorSuccess(DeviceManagerResponse<?> deviceManagerResponse) {
        DeviceManagerResponseError deviceManagerResponseError = new DeviceManagerResponseError();
        deviceManagerResponseError.setCode(0);
        deviceManagerResponseError.setDescription(STRING0);
        deviceManagerResponse.setError(deviceManagerResponseError);
    }

    private <T> DeviceManagerResponse<T> deviceManagerResponseSuccess(Class<T> t) {
        DeviceManagerResponse<T> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        return deviceManagerResponse;
    }
}
