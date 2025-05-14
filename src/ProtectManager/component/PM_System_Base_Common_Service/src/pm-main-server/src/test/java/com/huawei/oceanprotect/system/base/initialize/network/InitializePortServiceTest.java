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
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import com.huawei.oceanprotect.client.resource.manager.service.impl.NetWorkServiceImpl;
import com.huawei.oceanprotect.system.base.ResourceHelper;
import com.huawei.oceanprotect.system.base.dto.dorado.AllPortListResponseDto;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.ModifyLogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.bean.DeviceManagerServiceMockBean;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializePortServiceAbility;
import com.huawei.oceanprotect.system.base.initialize.network.beans.PortFactory;
import com.huawei.oceanprotect.system.base.initialize.network.beans.VlanPort;
import com.huawei.oceanprotect.system.base.model.BondPortPo;
import com.huawei.oceanprotect.system.base.model.LogicPortFilterParam;
import com.huawei.oceanprotect.system.base.model.ServicePortPo;
import com.huawei.oceanprotect.system.base.model.VlanPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPortRes;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.failovergroup.FailoverGroupResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.vlan.VlanInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RunningStatus;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.VlanPortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.DeviceService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.SystemService;
import com.huawei.oceanprotect.system.base.vo.DeviceInfo;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.bean.NetWorkIpRoutesInfo;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.bean.NetWorkRouteInfo;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.CommonUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;
import openbackup.system.base.sdk.storage.model.OceanStorageLogincalPortRes;
import openbackup.system.base.sdk.storage.model.StorageCommonRes;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 测试 DM端口添加情况
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-03
 */
@PrepareForTest(value = {CommonUtil.class})
@RunWith(PowerMockRunner.class)
public class InitializePortServiceTest {
    private final DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);

    private final InitNetworkConfigMapper initNetworkConfigMapper = Mockito.mock(InitNetworkConfigMapper.class);
    private final NetWorkPortService netWorkPortService = Mockito.mock(NetWorkPortService.class);
    private final NetworkService networkService = Mockito.mock(NetworkService.class);
    private final SystemService systemService = Mockito.mock(SystemService.class);
    private final DeviceService deviceService = Mockito.mock(DeviceService.class);
    private final PortFactory portFactory = Mockito.mock(PortFactory.class);
    private final com.huawei.oceanprotect.system.base.initialize.network.beans.BondPort bondPort =
            Mockito.mock(com.huawei.oceanprotect.system.base.initialize.network.beans.BondPort.class);
    private final VlanPort vlanPort = Mockito.mock(VlanPort.class);
    private final com.huawei.oceanprotect.system.base.initialize.network.beans.EthPort ethPort =
            Mockito.mock(com.huawei.oceanprotect.system.base.initialize.network.beans.EthPort.class);

    private final InitializePortServiceAbility initializePortService = new InitializePortServiceAbility();

    private final DeviceManagerServiceMockBean deviceManagerServiceMockBean = new DeviceManagerServiceMockBean();

    private final NetWorkServiceImpl netWorkServiceImpl = new NetWorkServiceImpl();

    private List<LogicPortDto> addLogicPortRequest = new ArrayList<>();

    private List<LogicPortAddRequest> addRequestList = new ArrayList<>();

    List<BondPortRes> bondPortResList = new ArrayList<>();

    @Before
    public void init() {
        Whitebox.setInternalState(initializePortService, "deployTypeService", deployTypeService);
        Mockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X8000);
        Whitebox.setInternalState(initializePortService, "portFactory", portFactory);
        Mockito.when(portFactory.createPort(any())).thenReturn(ethPort);
        Whitebox.setInternalState(ethPort, "systemService", systemService);
        Whitebox.setInternalState(ethPort, "netWorkPortService", netWorkPortService);
        Whitebox.setInternalState(initializePortService, "systemService", systemService);
        Whitebox.setInternalState(initializePortService, "netWorkPortService", netWorkPortService);
        Whitebox.setInternalState(initializePortService, "networkService", networkService);
        Whitebox.setInternalState(initializePortService, "initNetworkConfigMapper", initNetworkConfigMapper);
        List<InitConfigInfo> logicPortNameConfigList = new ArrayList<>();
        InitConfigInfo logicPortNameConfig = new InitConfigInfo();
        logicPortNameConfig.setInitType(Constants.LOGIC_PORTS_CREATED_BY_USER);
        logicPortNameConfig.setInitValue("[\"backB\",\"back4\",\"backA\"]");
        logicPortNameConfigList.add(logicPortNameConfig);

        List<InitConfigInfo> servicePortConfigList = new ArrayList<>();
        InitConfigInfo servicePortConfig = new InitConfigInfo();
        servicePortConfig.setInitType("backB");
        servicePortConfig.setInitValue("{\"name\":\"backB\",\"id\":\"4614219297495973895\",\"homePortType\":\"1\",\"vlan\":null,\"bondPort\":null}");
        servicePortConfigList.add(servicePortConfig);
        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString(), anyString())).thenReturn(logicPortNameConfigList, servicePortConfigList);
        given(netWorkPortService.queryEthPorts(any(), any())).willReturn(deviceManagerServiceMockBean.getDeviceManagerEth());
        DeviceManagerResponse<List<BondPortRes>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponse.setData(deviceManagerServiceMockBean.getBondPortResList());
        given(netWorkPortService.getBondPort(any(), any())).willReturn(deviceManagerResponse);
        given(systemService.getDeviceInfo()).willReturn(new DeviceInfo("wuyanzu", "zhendeshuai"));
        StorageCommonRes<List<OceanStorageLogincalPortRes>> storageCommonRes = new StorageCommonRes<>();
        storageCommonRes.setData(new ArrayList<>());
        given(deviceService.getLogicPort(any(), any())).willReturn(storageCommonRes);
        String addLogicPortRequestJson = ResourceHelper.getResourceAsString(getClass(),
                "AddLogicPortRequest.json");
        addLogicPortRequest = JSON.parseObject(addLogicPortRequestJson,
                new TypeReference<List<LogicPortDto>>() {
                });
        addLogicPortRequest.forEach(port -> {
            port.setRole(PortRole.SERVICE);
            port.setHomePortType(HomePortType.ETHERNETPORT);
        });

        String logicPortOfDbJson = ResourceHelper.getResourceAsString(getClass(),
                "LogicPortOfDb.json");
        List<ServicePortPo> logicPortOfDb = JSON.parseObject(logicPortOfDbJson,
                new TypeReference<List<ServicePortPo>>() {
                });

        String logicPortAddRequestListJson = ResourceHelper.getResourceAsString(getClass(),
                "EthPortList.json");
        List<EthPort> ethPortList = JSON.parseObject(logicPortAddRequestListJson,
                new TypeReference<List<EthPort>>() {
                });
        DeviceManagerResponse<List<EthPort>> listDeviceManagerResponse = new DeviceManagerResponse<>();
        listDeviceManagerResponse.setData(ethPortList);
        Mockito.when(netWorkPortService.queryEthPorts(any(), any())).thenReturn(listDeviceManagerResponse);

        String AddLogicPortRequestJson = ResourceHelper.getResourceAsString(getClass(),
                "LogicPortAddRequestList.json");
        addRequestList = JSON.parseObject(AddLogicPortRequestJson,
                new TypeReference<List<LogicPortAddRequest>>() {
                });
        addRequestList.forEach(port -> {
            port.setHomePortType(HomePortType.ETHERNETPORT);
            port.setRole(PortRole.SERVICE);
        });
        DeviceManagerResponse<List<LogicPortAddRequest>> listStorageCommonRes = new DeviceManagerResponse<>();
        listStorageCommonRes.setData(new ArrayList<>());
        given(netWorkPortService.queryLogicPorts(any(), any())).willReturn(listStorageCommonRes);
        DeviceManagerResponse<List<LogicPortAddRequest>> commonRes = new DeviceManagerResponse<>();
        commonRes.setData(addRequestList);
        Mockito.when(netWorkPortService.queryLogicPorts(any(), any())).thenReturn(commonRes);

        String bondPortListJson = ResourceHelper.getResourceAsString(getClass(),
                "BondPortList.json");
        bondPortResList = JSON.parseObject(bondPortListJson,
                new TypeReference<List<BondPortRes>>() {
                });
        bondPortResList.forEach(bondPortRes -> bondPortRes.setRunningStatus(RunningStatus.LINKUP));
        DeviceManagerResponse<List<BondPortRes>> b = new DeviceManagerResponse<>();
        b.setData(bondPortResList);
        Mockito.when(netWorkPortService.getBondPort(any(), any())).thenReturn(b);

        String vlanPortListJson = ResourceHelper.getResourceAsString(getClass(),
                "VlanPortList.json");
        List<VlanInfo> vlanInfoList = JSON.parseObject(vlanPortListJson,
                new TypeReference<List<VlanInfo>>() {
                });
        vlanInfoList.forEach(v -> v.setPortType(VlanPortType.BOND));
        DeviceManagerResponse<List<VlanInfo>> c = new DeviceManagerResponse<>();
        c.setData(vlanInfoList);
        Mockito.when(netWorkPortService.queryVlan(any(), any())).thenReturn(c);

        Map<String, List<NetWorkRouteInfo>> map = new HashMap<>();
        List<NetWorkRouteInfo> routesInfos1 = new ArrayList<>();
        routesInfos1.add(new NetWorkRouteInfo("1", "192.168.2.3", "255.255.255.255", "192.168.4.5"));
        map.put("192.168.111.111", routesInfos1);
        List<NetWorkRouteInfo> routesInfos2 = new ArrayList<>();
        routesInfos2.add(new NetWorkRouteInfo("1", "192.168.2.33", "255.255.255.255", "192.168.4.55"));
        map.put("192.168.111.111", routesInfos2);
        Mockito.when(networkService.getNetPlaneIpRouteList(any())).thenReturn(map);

        DeviceManagerResponse<List<PortRouteInfo>> response = new DeviceManagerResponse<>();
        List<PortRouteInfo> portRouteInfoList = new ArrayList<>();
        PortRouteInfo portRouteInfo1 = prepareRouteInfo1();
        portRouteInfoList.add(portRouteInfo1);
        PortRouteInfo portRouteInfo2 = prepareRouteInfo2();
        portRouteInfoList.add(portRouteInfo2);
        response.setData(portRouteInfoList);
        response.setError(new DeviceManagerResponseError(0, "", ""));
        Mockito.when(netWorkPortService.getRoute(any(), any(), any())).thenReturn(response);
    }

    @Test
    public void test_getNetPlaneIpList_should_success() {
        NetWorkConfigInfo netWorkConfigInfo1 = JSON.parseObject("{\"nodeId\":\"node-0\",\"logic_ip_list\":[{\"ip\":\"192.168.115.12\",\"mask\":\"255.255.0.0\"}],\"ips_route_table\":[]}", NetWorkConfigInfo.class);
        NetWorkConfigInfo netWorkConfigInfo2 = JSON.parseObject("{\"nodeId\":\"node-1\",\"logic_ip_list\":[{\"ip\":\"192.168.115.13\",\"mask\":\"255.255.0.0\"}],\"ips_route_table\":[]}", NetWorkConfigInfo.class);
        List<NetWorkConfigInfo> netWorkConfigInfoList = new ArrayList<>();
        netWorkConfigInfoList.add(netWorkConfigInfo1);
        netWorkConfigInfoList.add(netWorkConfigInfo2);
        Assert.assertEquals(2 , netWorkServiceImpl.getNetPlaneIpList(netWorkConfigInfoList).size());
    }

    private PortRouteInfo prepareRouteInfo1() {
        PortRouteInfo info1 = new PortRouteInfo();
        info1.setRouteType(RouteType.MASTER);
        info1.setDestination("192.168.2.3");
        info1.setMask("255.255.255.255");
        info1.setGateway("192.168.4.5");
        return info1;
    }
    private PortRouteInfo prepareRouteInfo2() {
        PortRouteInfo info2 = new PortRouteInfo();
        info2.setRouteType(RouteType.MASTER);
        info2.setDestination("192.168.2.33");
        info2.setMask("255.255.255.255");
        info2.setGateway("192.168.4.55");
        return info2;
    }


    /**
     * 用例场景：处理vlan类型逻辑端口，添加保存到数据库
     * 前置条件：NA
     * 检查点：处理成功无报错
     */
    @Test
    public void test_handle_vlan_logic_ports_success() {
        addLogicPortRequest.get(0).setHomePortType(HomePortType.VLAN);
        addLogicPortRequest.get(0).setRole(PortRole.TRANSLATE);
        addLogicPortRequest.get(0).getVlan().setPortType(VlanPortType.BOND);
        DeviceManagerResponse<BondPortRes> bondDeviceManagerResponse = new DeviceManagerResponse<>();
        bondDeviceManagerResponse.setData(new BondPortRes());
        Mockito.when(netWorkPortService.addBondPort(anyString(), anyString(), any())).thenReturn(bondDeviceManagerResponse);
        DeviceManagerResponse<VlanInfo> vlanInfoDeviceManagerResponse = new DeviceManagerResponse<>();
        vlanInfoDeviceManagerResponse.setData(new VlanInfo());
        Mockito.when(netWorkPortService.addVlan(anyString(), anyString(), any())).thenReturn(vlanInfoDeviceManagerResponse);
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        DeviceManagerResponse<BondPortRes> response = new DeviceManagerResponse<>();
        response.setData(bondPortResList.get(0));
        Mockito.when(netWorkPortService.addBondPort(anyString(), anyString(), any())).thenReturn(response);

        DeviceManagerResponse<FailoverGroupResponse> response1 = new DeviceManagerResponse<>();
        FailoverGroupResponse failoverGroupResponse = new FailoverGroupResponse();
        failoverGroupResponse.setId("wuyanzu");
        response1.setData(failoverGroupResponse);
        Mockito.when(netWorkPortService.createFailovergroup(anyString(), anyString(), any())).thenReturn(response1);

        DeviceManagerResponse response2 = new DeviceManagerResponse<>();
        Mockito.when(netWorkPortService.addMemberOfFailovergroup(anyString(), anyString(), any())).thenReturn(response2);

        PowerMockito.mockStatic(CommonUtil.class);
        BondPortPo bondPort1 = new BondPortPo();
        bondPort1.setId("");
        addLogicPortRequest.get(0).setBondPort(bondPort1);
        addLogicPortRequest.get(0).setRole(PortRole.SERVICE);
        initializePortService.handleLogicPort(addLogicPortRequest.get(0));
        verify(netWorkPortService, times(1)).addLogicPort(anyString(), anyString(), any());
    }

    /**
     * 用例场景：处理绑定端口类型逻辑端口，添加保存到数据库
     * 前置条件：NA
     * 检查点：处理成功无报错
     */
    @Test
    public void test_handle_bond_logic_ports_success() {
        addLogicPortRequest.get(2).setHomePortType(HomePortType.BINDING);
        addLogicPortRequest.get(2).setRole(PortRole.SERVICE);
        DeviceManagerResponse<BondPortRes> bondDeviceManagerResponse = new DeviceManagerResponse<>();
        bondDeviceManagerResponse.setData(new BondPortRes());
        Mockito.when(netWorkPortService.addBondPort(anyString(), anyString(), any())).thenReturn(bondDeviceManagerResponse);
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        DeviceManagerResponse<BondPortRes> response = new DeviceManagerResponse<>();
        response.setData(bondPortResList.get(0));
        Mockito.when(netWorkPortService.addBondPort(anyString(), anyString(), any())).thenReturn(response);

        PowerMockito.mockStatic(CommonUtil.class);

        DeviceManagerResponse<FailoverGroupResponse> response1 = new DeviceManagerResponse<>();
        FailoverGroupResponse failoverGroupResponse = new FailoverGroupResponse();
        failoverGroupResponse.setId("wuyanzu");
        response1.setData(failoverGroupResponse);
        Mockito.when(netWorkPortService.createFailovergroup(anyString(), anyString(), any())).thenReturn(response1);

        DeviceManagerResponse response2 = new DeviceManagerResponse<>();
        Mockito.when(netWorkPortService.addMemberOfFailovergroup(anyString(), anyString(), any())).thenReturn(response2);

        PowerMockito.mockStatic(CommonUtil.class);
        BondPortPo bondPort1 = new BondPortPo();
        bondPort1.setId("");
        addLogicPortRequest.get(2).setBondPort(bondPort1);
        addLogicPortRequest.get(2).setRole(PortRole.SERVICE);
        initializePortService.handleLogicPort(addLogicPortRequest.get(2));
        verify(netWorkPortService, times(1)).addLogicPort(anyString(), anyString(), any());
    }

    /**
     * 用例场景：处理以太网类型逻辑端口，添加保存到数据库
     * 前置条件：NA
     * 检查点：处理成功无报错
     */
    @Test
    public void test_handle_eth_logic_ports_success() {
        addLogicPortRequest.get(1).setHomePortType(HomePortType.ETHERNETPORT);
        addLogicPortRequest.get(1).setRole(PortRole.SERVICE);
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        DeviceManagerResponse<FailoverGroupResponse> response1 = new DeviceManagerResponse<>();
        FailoverGroupResponse failoverGroupResponse = new FailoverGroupResponse();
        failoverGroupResponse.setId("wuyanzu");
        response1.setData(failoverGroupResponse);
        Mockito.when(netWorkPortService.createFailovergroup(anyString(), anyString(), any())).thenReturn(response1);

        DeviceManagerResponse response2 = new DeviceManagerResponse<>();
        Mockito.when(netWorkPortService.addMemberOfFailovergroup(anyString(), anyString(), any())).thenReturn(response2);

        PowerMockito.mockStatic(CommonUtil.class);
        BondPortPo bondPort1 = new BondPortPo();
        bondPort1.setId("");
        addLogicPortRequest.get(1).setBondPort(bondPort1);
        addLogicPortRequest.get(1).setRole(PortRole.SERVICE);
        initializePortService.handleLogicPort(addLogicPortRequest.get(1));
        verify(netWorkPortService, times(1)).addLogicPort(anyString(), anyString(), any());
    }

    /**
     * 用例场景：模拟初始化复用逻辑端口场景，失败
     * 前置条件：NA
     * 检查点：处理成功无报错
     */
    @Test
    public void test_init_reuse_logic_ports_fail() {
        addLogicPortRequest.get(1).setHomePortType(HomePortType.ETHERNETPORT);
        addLogicPortRequest.get(1).setRole(PortRole.SERVICE);
        addLogicPortRequest.get(1).setName("back4");
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString(), anyString())).thenReturn(new ArrayList<>());

        DeviceManagerResponse<List<LogicPortAddRequest>> queryLogicDeviceManagerResponse =
                new DeviceManagerResponse<>();
        queryLogicDeviceManagerResponse.setData(addRequestList);
        Mockito.when(netWorkPortService.queryLogicPorts(anyString(), anyString())).thenReturn(queryLogicDeviceManagerResponse);
        initializePortService.handleLogicPort(addLogicPortRequest.get(1));
        verify(netWorkPortService, times(1)).queryLogicPorts(anyString(), anyString());
    }

    /**
     * 用例场景：模拟修改网络复用逻辑端口场景，复用pm创建的逻辑端口，失败
     * 前置条件：NA
     * 检查点：处理成功无报错
     */
    @Test
    public void test_modify_network_reuse_logic_ports_fail() {
        addLogicPortRequest.get(1).setHomePortType(HomePortType.ETHERNETPORT);
        addLogicPortRequest.get(1).setRole(PortRole.SERVICE);
        addLogicPortRequest.get(1).setId("4614219297495973894");
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        List<InitConfigInfo> list = new ArrayList<>();
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitValue("[\"backA\",\"backB\"]");
        list.add(initConfigInfo);
        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString(), anyString())).thenReturn(list);

        DeviceManagerResponse<List<LogicPortAddRequest>> queryLogicDeviceManagerResponse =
                new DeviceManagerResponse<>();
        queryLogicDeviceManagerResponse.setData(addRequestList);
        Mockito.when(netWorkPortService.queryLogicPorts(anyString(), anyString())).thenReturn(queryLogicDeviceManagerResponse);
        initializePortService.handleLogicPort(addLogicPortRequest.get(1));
        verify(netWorkPortService, times(1)).queryLogicPorts(anyString(), anyString());
    }

/**
     * 用例场景：模拟修改网络复用逻辑端口场景，复用底座创建的逻辑端口，成功
     * 前置条件：NA
     * 检查点：处理成功无报错
     */
    @Test
    public void test_modify_network_reuse_logic_ports_success() {
        addLogicPortRequest.get(1).setHomePortType(HomePortType.ETHERNETPORT);
        addLogicPortRequest.get(1).setRole(PortRole.SERVICE);
        addLogicPortRequest.get(1).setId("4614219297495973896");
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        List<InitConfigInfo> list = new ArrayList<>();
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitValue("[\"backA\",\"backB\"]");
        list.add(initConfigInfo);
        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString(), anyString())).thenReturn(list);

        DeviceManagerResponse<List<LogicPortAddRequest>> queryLogicDeviceManagerResponse =
                new DeviceManagerResponse<>();
        queryLogicDeviceManagerResponse.setData(addRequestList);
        Mockito.when(netWorkPortService.queryLogicPorts(anyString(), anyString())).thenReturn(queryLogicDeviceManagerResponse);
        initializePortService.handleLogicPort(addLogicPortRequest.get(1));
        verify(netWorkPortService, times(1)).queryLogicPorts(anyString(), anyString());
    }

    /**
     * 用例场景：复用逻辑端口成功
     * 前置条件：NA
     * 检查点：处理成功无报错
     */
    @Test
    public void test_handle_logic_ports_and_reuse_success() {
        addLogicPortRequest.get(0).setHomePortId("287136175");
        addLogicPortRequest.get(0).setHomePortType(HomePortType.ETHERNETPORT);
        addLogicPortRequest.get(0).setRole(PortRole.SERVICE);
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);
        initializePortService.handleLogicPort(addLogicPortRequest.get(0));
        verify(netWorkPortService, times(1)).addLogicPort(anyString(), anyString(), any());
    }

    /**
     * 用例场景：查询所有端口
     * 前置条件：NA
     * 检查点：查询成功
     */
    @Test
    public void test_get_all_ports_success() {
        Mockito.when(networkService.isUpdateFromBeforeVersionSix()).thenReturn(true);
        NetWorkLogicIp netWorkLogicIp = new NetWorkLogicIp();
        netWorkLogicIp.setIp("192.168.111.111");
        netWorkLogicIp.setMask("255.255.0.0");

        NetWorkLogicIp netWorkLogicIp1 = new NetWorkLogicIp();
        netWorkLogicIp1.setIp("192.168.102.208");
        netWorkLogicIp1.setMask("255.255.0.0");

        NetWorkLogicIp netWorkLogicIp2 = new NetWorkLogicIp();
        netWorkLogicIp2.setIp("192.168.102.209");
        netWorkLogicIp2.setMask("255.255.0.0");

        NetWorkLogicIp netWorkLogicIp3 = new NetWorkLogicIp();
        netWorkLogicIp3.setIp("192.168.111.113");
        netWorkLogicIp3.setMask("255.255.0.0");
        ArrayList<NetWorkLogicIp> list1 = new ArrayList<>();
        list1.add(netWorkLogicIp);
        list1.add(netWorkLogicIp1);
        list1.add(netWorkLogicIp2);
        list1.add(netWorkLogicIp3);

        NetWorkConfigInfo netWorkConfigInfo = new NetWorkConfigInfo();
        netWorkConfigInfo.setNodeId("wuyanzu");
        netWorkConfigInfo.setLogicIpList(list1);
        DeviceNetworkInfo deviceNetworkInfo = new DeviceNetworkInfo();
        ArrayList<NetWorkConfigInfo> list = new ArrayList<>();
        list.add(netWorkConfigInfo);
        deviceNetworkInfo.setBackupConfig(list);
        Mockito.when(networkService.getDeviceNetworkInfo()).thenReturn(deviceNetworkInfo);
        List<String> list2 = Arrays.asList("192.168.111.111", "192.168.102.208", "192.168.102.209", "192.168.111.113");
        Mockito.when(networkService.getNetPlaneIp(any())).thenReturn(list2);

        String AddLogicPortRequestJson = ResourceHelper.getResourceAsString(getClass(),
                "LogicPortAddRequestList.json");
        addRequestList = JSON.parseObject(AddLogicPortRequestJson,
                new TypeReference<List<LogicPortAddRequest>>() {
                });
        addRequestList.forEach(a -> {
            a.setHomePortType(HomePortType.ETHERNETPORT);
            if (a.getIpv4Addr().equals("192.168.102.208")) {
                a.setHomePortType(HomePortType.ETHERNETPORT);
            }
            if (a.getIpv4Addr().equals("192.168.102.209")) {
                a.setHomePortType(HomePortType.BINDING);
            }
            if (a.getIpv4Addr().equals("192.168.111.111")) {
                a.setHomePortType(HomePortType.BINDING);
            }
            if (a.getIpv4Addr().equals("192.168.111.113")) {
                a.setHomePortType(HomePortType.VLAN);
            }
        });
        DeviceManagerResponse<List<LogicPortAddRequest>> commonRes = new DeviceManagerResponse<>();
        commonRes.setData(addRequestList);
        Mockito.when(netWorkPortService.queryLogicPorts(any(), any())).thenReturn(commonRes);
        AllPortListResponseDto ports = initializePortService.getPorts(new LogicPortFilterParam());
        Assert.assertEquals(ports.getLogicPortDtoList().size(), 1);
    }

    /**
     * 用例场景：根据条件查询端口
     * 前置条件：NA
     * 检查点：查询成功
     */
    @Test
    public void test_get_condition_ports_success() {
        NetWorkLogicIp netWorkLogicIp = new NetWorkLogicIp();
        netWorkLogicIp.setIp("192.168.111.111");
        netWorkLogicIp.setMask("255.255.0.0");

        NetWorkLogicIp netWorkLogicIp1 = new NetWorkLogicIp();
        netWorkLogicIp1.setIp("192.168.102.208");
        netWorkLogicIp1.setMask("255.255.0.0");

        NetWorkLogicIp netWorkLogicIp2 = new NetWorkLogicIp();
        netWorkLogicIp2.setIp("192.168.102.209");
        netWorkLogicIp2.setMask("255.255.0.0");

        NetWorkLogicIp netWorkLogicIp3 = new NetWorkLogicIp();
        netWorkLogicIp3.setIp("192.168.111.113");
        netWorkLogicIp3.setMask("255.255.0.0");
        ArrayList<NetWorkLogicIp> list1 = new ArrayList<>();
        list1.add(netWorkLogicIp);
        list1.add(netWorkLogicIp1);
        list1.add(netWorkLogicIp2);
        list1.add(netWorkLogicIp3);

        NetWorkConfigInfo netWorkConfigInfo = new NetWorkConfigInfo();
        netWorkConfigInfo.setNodeId("wuyanzu");
        netWorkConfigInfo.setLogicIpList(list1);
        netWorkConfigInfo.setIpRouteList(prepareIpRoutesList());
        DeviceNetworkInfo deviceNetworkInfo = new DeviceNetworkInfo();
        ArrayList<NetWorkConfigInfo> list = new ArrayList<>();
        list.add(netWorkConfigInfo);
        deviceNetworkInfo.setBackupConfig(list);
        Mockito.when(networkService.getDeviceNetworkInfo()).thenReturn(deviceNetworkInfo);
        // Mockito.when(netWorkPortService.queryLogicPorts(any(), any())).thenReturn();
        List<String> list2 = Arrays.asList("192.168.111.111", "192.168.102.208", "192.168.102.209", "192.168.111.113");
        Mockito.when(networkService.getNetPlaneIp(any())).thenReturn(list2);

        LogicPortFilterParam condition = new LogicPortFilterParam();
        condition.setPortName("backB");
        condition.setPortId("212121");
        AllPortListResponseDto ports = initializePortService.getPorts(condition);
        Assert.assertEquals(ports.getLogicPortDtoList().size(), 3);
    }

    private List<NetWorkIpRoutesInfo> prepareIpRoutesList() {
        List<NetWorkIpRoutesInfo> res = new ArrayList<>();
        NetWorkIpRoutesInfo info1 = new NetWorkIpRoutesInfo();
        info1.setIp("192.168.111.111");
        List<NetWorkRouteInfo> routeInfos1 = new ArrayList<>();
        NetWorkRouteInfo netWorkRouteInfo1 = new NetWorkRouteInfo();
        netWorkRouteInfo1.setType("1");
        netWorkRouteInfo1.setDestination("192.168.2.3");
        netWorkRouteInfo1.setMask("255.255.255.255");
        netWorkRouteInfo1.setGateway("192.168.4.5");
        routeInfos1.add(netWorkRouteInfo1);
        info1.setRoutes(routeInfos1);

        NetWorkIpRoutesInfo info2 = new NetWorkIpRoutesInfo();
        info2.setIp("192.168.111.111");
        List<NetWorkRouteInfo> routeInfos2 = new ArrayList<>();
        NetWorkRouteInfo netWorkRouteInfo2 = new NetWorkRouteInfo();
        netWorkRouteInfo2.setType("1");
        netWorkRouteInfo2.setDestination("192.168.2.33");
        netWorkRouteInfo2.setMask("255.255.255.255");
        netWorkRouteInfo2.setGateway("192.168.4.55");
        routeInfos2.add(netWorkRouteInfo2);
        info2.setRoutes(routeInfos2);

        res.add(info1);
        res.add(info2);
        return res;
    }

    /**
     * 用例场景：修改逻辑端口
     * 前置条件：NA
     * 检查点：修改逻辑端口成功
     */
    @Test
    public void test_modify_ports_success() {
        ModifyLogicPortDto modifyLogicPortDto = new ModifyLogicPortDto();
        modifyLogicPortDto.setId("4614219297495973895");
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.modifyLogicPortById(anyString(), anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        List<InitConfigInfo> logicPortNameConfigList = new ArrayList<>();
        InitConfigInfo logicPortNameConfig = new InitConfigInfo();
        logicPortNameConfig.setInitType(Constants.LOGIC_PORTS_CREATED_BY_USER);
        logicPortNameConfig.setInitValue("[\"backB\",\"back4\",\"backA\"]");
        logicPortNameConfigList.add(logicPortNameConfig);

        List<InitConfigInfo> servicePortConfigList = new ArrayList<>();
        InitConfigInfo servicePortConfig = new InitConfigInfo();
        servicePortConfig.setInitType("backB");
        servicePortConfig.setInitValue("{\"name\":\"backB\",\"id\":\"4614219297495973895\",\"homePortType\":\"1\"," +
                "\"vlan\":null,\"bondPort\":null, \"role\":11}");
        servicePortConfigList.add(servicePortConfig);
        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString(), anyString())).thenReturn(servicePortConfigList, servicePortConfigList, servicePortConfigList, logicPortNameConfigList);
        Mockito.when(networkService.isUpdateFromBeforeVersionSix()).thenReturn(true);
        initializePortService.modifyLogicPort("backB", modifyLogicPortDto);
        verify(netWorkPortService, times(1)).modifyLogicPortById(anyString(), anyString(), anyString() , any());
    }

    /**
     * 用例场景：修改逻辑端口,并且保存端口角色
     * 前置条件：NA
     * 检查点：修改逻辑端口成功，保存端口角色成功
     */
    @Test
    public void test_modify_ports_and_save_port_role_success() {
        ModifyLogicPortDto modifyLogicPortDto = new ModifyLogicPortDto();
        modifyLogicPortDto.setId("4614219297495973895");
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.modifyLogicPortById(anyString(), anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);

        List<InitConfigInfo> logicPortNameConfigList = new ArrayList<>();
        InitConfigInfo logicPortNameConfig = new InitConfigInfo();
        logicPortNameConfig.setInitType(Constants.LOGIC_PORTS_CREATED_BY_USER);
        logicPortNameConfig.setInitValue("[\"backB\",\"back4\",\"backA\"]");
        logicPortNameConfigList.add(logicPortNameConfig);

        List<InitConfigInfo> servicePortConfigList = new ArrayList<>();
        InitConfigInfo servicePortConfig = new InitConfigInfo();
        servicePortConfig.setInitType("backB");
        servicePortConfig.setInitValue("{\"name\":\"backB\",\"id\":\"4614219297495973895\",\"homePortType\":\"1\",\"vlan\":null,\"bondPort\":null}");
        servicePortConfigList.add(servicePortConfig);
        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString(), anyString())).thenReturn(servicePortConfigList, servicePortConfigList, servicePortConfigList, logicPortNameConfigList);
        Mockito.when(networkService.isUpdateFromBeforeVersionSix()).thenReturn(true);
        Mockito.when(networkService.getDeviceNetworkInfo()).thenReturn(new DeviceNetworkInfo());
        Mockito.when(networkService.getNetPlaneIp(any())).thenReturn(Arrays.asList("192.168.111.117"));
        initializePortService.modifyLogicPort("backB", modifyLogicPortDto);
        verify(netWorkPortService, times(1)).modifyLogicPortById(anyString(), anyString(), anyString() , any());
    }

    /**
     * 用例场景：删除业务端口
     * 前置条件：NA
     * 检查点：删除业务端口成功
     */
    @Test
    public void test_delete_port_success() {
        List<InitConfigInfo> logicPortNameConfigList = new ArrayList<>();
        InitConfigInfo logicPortNameConfig = new InitConfigInfo();
        logicPortNameConfig.setInitType(Constants.LOGIC_PORTS_CREATED_BY_USER);
        logicPortNameConfig.setInitValue("[\"backB\",\"back4\",\"backA\"]");
        logicPortNameConfigList.add(logicPortNameConfig);

        List<InitConfigInfo> reuseLogicPortNameConfigList = new ArrayList<>();
        InitConfigInfo reuseLogicPortNameConfig = new InitConfigInfo();
        reuseLogicPortNameConfig.setInitType(Constants.REUSE_LOGIC_PORTS);
        reuseLogicPortNameConfig.setInitValue("[]");
        reuseLogicPortNameConfigList.add(reuseLogicPortNameConfig);

        List<InitConfigInfo> servicePortConfigList = new ArrayList<>();
        InitConfigInfo servicePortConfig = new InitConfigInfo();
        servicePortConfig.setInitType("backB");
        servicePortConfig.setInitValue("{\"name\":\"backB\",\"id\":\"4614219297495973895\",\"homePortType\":\"1\",\"vlan\":null,\"bondPort\":null}");
        servicePortConfigList.add(servicePortConfig);
        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString() ,anyString())).thenReturn(reuseLogicPortNameConfigList, servicePortConfigList, logicPortNameConfigList, logicPortNameConfigList);

        DeviceManagerResponse objectDeviceManagerResponse = new DeviceManagerResponse<>();
        Mockito.when(netWorkPortService.deleteFailovergroup(anyString(), anyString(), anyString())).thenReturn(objectDeviceManagerResponse);

        initializePortService.deleteLogicPort("backB");
        verify(netWorkPortService, times(1)).deleteLogicPort(anyString(), anyString(), anyString());
    }

    /**
     * 用例场景：删除业务端口
     * 前置条件：NA
     * 检查点：删除业务端口成功
     */
    @Test
    public void test_only_delete_port_of_db_success() {
        List<InitConfigInfo> logicPortNameConfigList = new ArrayList<>();
        InitConfigInfo logicPortNameConfig = new InitConfigInfo();
        logicPortNameConfig.setInitType(Constants.LOGIC_PORTS_CREATED_BY_USER);
        logicPortNameConfig.setInitValue("[\"backB\",\"back4\",\"backA\"]");
        logicPortNameConfigList.add(logicPortNameConfig);

        List<InitConfigInfo> reuseLogicPortNameConfigList = new ArrayList<>();
        InitConfigInfo reuseLogicPortNameConfig = new InitConfigInfo();
        reuseLogicPortNameConfig.setInitType(Constants.REUSE_LOGIC_PORTS);
        reuseLogicPortNameConfig.setInitValue("[\"backB\",\"back4\",\"backA\"]");
        reuseLogicPortNameConfigList.add(logicPortNameConfig);

        Mockito.when(initNetworkConfigMapper.queryInitConfigByEsnAndType(anyString() ,anyString())).thenReturn(reuseLogicPortNameConfigList, reuseLogicPortNameConfigList, logicPortNameConfigList);

        initializePortService.deleteLogicPort("backB");
        Assert.assertEquals(1, logicPortNameConfigList.size());
    }

    /**
     * 用例场景：删除dm端口
     * 前置条件：NA
     * 检查点：删除dm端口成功
     */
    @Test
    public void test_delete_dm_port_success() throws Exception {
        ServicePortPo existServicePort = new ServicePortPo();
        existServicePort.setHomePortType(HomePortType.VLAN);
        VlanPo vlan = new VlanPo();
        vlan.setPortType(VlanPortType.BOND);
        existServicePort.setVlan(vlan);
        Whitebox.invokeMethod(initializePortService, "deletePhysicPort", existServicePort);

        existServicePort.setHomePortType(HomePortType.BINDING);
        existServicePort.setBondPort(new BondPortPo());
        Whitebox.invokeMethod(initializePortService, "deletePhysicPort", existServicePort);
    }

    /**
     * 用例场景：添加绑定类型抛出指定异常
     * 前置条件：NA
     * 检查点：抛出LegoCheckedException
     */
    @Test(expected = LegoCheckedException.class)
    public void test_add_bond_port_success() throws Exception {
        LogicPortDto logicPort = new LogicPortDto();
        BondPortPo bondPort = new BondPortPo();
        bondPort.setPortNameList(Arrays.asList("CTE0.A.IOM0.P1", "CTE0.A.IOM0.P2"));
        bondPort.setMtu("1600");
        logicPort.setBondPort(bondPort);
        logicPort.setRole(PortRole.MANAGEMENT);
        DeviceManagerResponse<BondPortRes> bondPortResDeviceManagerResponse = new DeviceManagerResponse<>();
        bondPortResDeviceManagerResponse.setData(new BondPortRes());
        Mockito.when(netWorkPortService.addBondPort(anyString(), anyString(), any())).thenReturn(bondPortResDeviceManagerResponse);
        LogicPortDto logicPortDto = new LogicPortDto();
        logicPortDto.setRole(PortRole.TRANSLATE);
        DeviceManagerResponse<BondPort> logicDeviceManagerResponse = new DeviceManagerResponse<>();
        logicDeviceManagerResponse.setData(new BondPort());
        Mockito.when(netWorkPortService.addLogicPort(anyString(), anyString(), any())).thenReturn(logicDeviceManagerResponse);
        Mockito.doThrow(new LegoCheckedException("")).when(netWorkPortService).modifyBondPort(anyString(),
                anyString(), anyString(), any());
        PowerMockito.mockStatic(CommonUtil.class);

        Whitebox.invokeMethod(initializePortService, "addBondTypeLogicPort", logicPort);
    }

    /**
     * 用例场景：校验绑定端口名称和端口列表，要么都相同，是复用；要么都不同，是新建；其他情况合理报错
     * 前置条件：测试的2个逻辑端口都应该是绑定端口
     * 检查点：检查成功
     */
    @Test
    public void test_checkIsValidPortNameAndPortNameList_should_success() {
        LogicPortDto logicPortDto1 = prepareLogicPortDto();
        ServicePortPo servicePortPo = new ServicePortPo();
        BondPortPo portPo = new BondPortPo();
        portPo.setName("bondPortName1");
        List<String> portNameList = new ArrayList<>();
        portNameList.add("CTE0.A.IOM0.P2");
        portNameList.add("CTE0.A.IOM0.P3");
        portPo.setPortNameList(portNameList);
        servicePortPo.setBondPort(portPo);
        initializePortService.checkIsValidPortNameAndPortNameList(logicPortDto1, servicePortPo);
        Assert.assertEquals(logicPortDto1.getBondPort().getName(), "bondPortName1");
    }

    /**
     * 用例场景：校验绑定端口名称和端口列表，绑定端口名称不同但列表相同的情况，应该报错
     * 前置条件：测试的2个逻辑端口都应该是绑定端口
     * 检查点：检查成功
     */
    @Test
    public void test_checkIsValidPortNameAndPortNameList_should_throw_LegoCheckedException_scene1() {
        LogicPortDto logicPortDto1 = prepareLogicPortDto();
        ServicePortPo servicePortPo = new ServicePortPo();
        BondPortPo portPo = new BondPortPo();
        portPo.setName("bondPortName2");
        List<String> portNameList = new ArrayList<>();
        portNameList.add("CTE0.A.IOM0.P2");
        portNameList.add("CTE0.A.IOM0.P3");
        portPo.setPortNameList(portNameList);
        servicePortPo.setBondPort(portPo);
        Assert.assertThrows(LegoCheckedException.class,
            () -> initializePortService.checkIsValidPortNameAndPortNameList(logicPortDto1, servicePortPo));
    }

    /**
     * 用例场景：校验绑定端口名称和端口列表，绑定端口名称相同但列表不同的情况，应该报错
     * 前置条件：测试的2个逻辑端口都应该是绑定端口
     * 检查点：检查成功
     */
    @Test
    public void test_checkIsValidPortNameAndPortNameList_should_throw_LegoCheckedException_scene2() {
        LogicPortDto logicPortDto1 = prepareLogicPortDto();
        ServicePortPo servicePortPo = new ServicePortPo();
        BondPortPo portPo = new BondPortPo();
        portPo.setName("bondPortName1");
        List<String> portNameList = new ArrayList<>();
        portNameList.add("CTE0.A.IOM0.P0");
        portNameList.add("CTE0.A.IOM0.P3");
        portPo.setPortNameList(portNameList);
        servicePortPo.setBondPort(portPo);
        Assert.assertThrows(LegoCheckedException.class,
            () -> initializePortService.checkIsValidPortNameAndPortNameList(logicPortDto1, servicePortPo));
    }

    public LogicPortDto prepareLogicPortDto() {
        LogicPortDto logicPortDto = new LogicPortDto();
        BondPortPo bondPortPo = new BondPortPo();
        bondPortPo.setName("bondPortName1");
        List<String> portNameList = new ArrayList<>();
        portNameList.add("CTE0.A.IOM0.P2");
        portNameList.add("CTE0.A.IOM0.P3");
        bondPortPo.setPortNameList(portNameList);
        logicPortDto.setBondPort(bondPortPo);
        return logicPortDto;
    }
}
