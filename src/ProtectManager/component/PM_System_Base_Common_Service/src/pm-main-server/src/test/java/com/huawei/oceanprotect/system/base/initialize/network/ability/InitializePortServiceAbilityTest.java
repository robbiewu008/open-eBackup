/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.ability;

import static org.mockito.ArgumentMatchers.any;

import com.alibaba.fastjson.JSON;

import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.bean.NetWorkRouteInfo;
import openbackup.system.base.service.NetworkService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author z00893213
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2025-03-05
 */
@RunWith(PowerMockRunner.class)
public class InitializePortServiceAbilityTest {
    @InjectMocks
    private InitializePortServiceAbility initializePortServiceAbility;

    @Mock
    private NetworkService networkService;

    @Test
    public void test_getAllNetWorkIpRouteMap_should_success() {
        DeviceNetworkInfo deviceNetworkInfo = prepareDeviceNetworkInfo();
        NetWorkLogicIp netWorkLogicIp = new NetWorkLogicIp();
        netWorkLogicIp.setIp("192.168.117.214");
        netWorkLogicIp.setMask("255.255.0.0");
        List<NetWorkLogicIp> netWorkLogicIps = Collections.singletonList(netWorkLogicIp);
        PowerMockito.when(networkService.getNetPlaneIpList(any())).thenReturn(netWorkLogicIps);
        Map<String, List<NetWorkRouteInfo>> allNetPlaneIpRouteMap = prepareAllNetPlaneIpRouteMap();
        PowerMockito.when(networkService.getNetPlaneIpRouteList(any())).thenReturn(allNetPlaneIpRouteMap);

        initializePortServiceAbility.getAllNetWorkIpRouteMap(deviceNetworkInfo);
        Mockito.verify(networkService, Mockito.times(3)).getNetPlaneIpList(any());
    }

    private Map<String, List<NetWorkRouteInfo>> prepareAllNetPlaneIpRouteMap() {
        Map<String, List<NetWorkRouteInfo>> allNetPlaneIpRouteMap = new HashMap<>();
        List<NetWorkRouteInfo> netWorkRouteInfoList = new ArrayList<>();
        netWorkRouteInfoList.add(new NetWorkRouteInfo());
        allNetPlaneIpRouteMap.put("192.168.115.12", netWorkRouteInfoList);
        return allNetPlaneIpRouteMap;
    }

    public DeviceNetworkInfo prepareDeviceNetworkInfo() {
        DeviceNetworkInfo deviceNetworkInfo = JSON.parseObject(
            "{\"backupConfig\":[{\"nodeId\":\"node-0\",\"logic_ip_list\":[{\"ip\":\"192.168.115.12\",\"mask\":\"255.255.0.0\"}],\"ips_route_table\":[]},"
                + "{\"nodeId\":\"node-1\",\"logic_ip_list\":[{\"ip\":\"192.168.115.13\",\"mask\":\"255.255.0.0\"}],\"ips_route_table\":[]}],"
                + "\"archiveConfig\":[{\"nodeId\":\"node-0\",\"logic_ip_list\":[{\"ip\":\"8.42.115.60\",\"mask\":\"255.255.224.0\"}],\"ips_route_table\":[]},"
                + "{\"nodeId\":\"node-1\",\"logic_ip_list\":[{\"ip\":\"8.42.115.61\",\"mask\":\"255.255.224.0\"}],\"ips_route_table\":[]}],\"replicationConfig\":[]}",
            DeviceNetworkInfo.class);
        return deviceNetworkInfo;
    }
}