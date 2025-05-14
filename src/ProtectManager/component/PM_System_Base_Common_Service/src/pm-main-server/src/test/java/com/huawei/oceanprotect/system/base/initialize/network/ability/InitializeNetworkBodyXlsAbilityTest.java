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

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.ResourceHelper;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkCfg;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.NetPlaneType;
import com.huawei.oceanprotect.system.base.model.BondPortPo;
import com.huawei.oceanprotect.system.base.model.NetworkInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPortRes;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.SystemService;
import com.huawei.oceanprotect.system.base.vo.DeviceInfo;

import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;
import org.springframework.web.multipart.MultipartFile;

import java.util.ArrayList;
import java.util.List;

/**
 * 功能描述
 *
 * @since 2024-03-19
 */
@RunWith(PowerMockRunner.class)
public class InitializeNetworkBodyXlsAbilityTest {
    @InjectMocks
    private InitializeNetworkBodyXlsAbility initializeNetworkBodyXlsAbility;

    @Mock
    private DeployTypeService deployTypeService;

    @Mock
    private InitializePortService initializePortService;

    @Mock
    private SystemService systemService;

    @Mock
    private NetWorkPortService netWorkPortService;

    @Mock
    private ClusterBasicService clusterBasicService;

    @Test
    public void test_fill_logic_port_type() {
        InitNetworkCfg initNetworkCfg1 = new InitNetworkCfg();
        InitNetworkCfg initNetworkCfg2 = new InitNetworkCfg();
        initNetworkCfg1.setVlanID("2500");
        initNetworkCfg2.setVlanID("-");
        initNetworkCfg1.setPortType(PortType.ETHERNET.getPortType());
        initNetworkCfg2.setPortType(PortType.BINDING.getPortType());
        LogicPortDto logicPortDto1 = new LogicPortDto();
        LogicPortDto logicPortDto2 = new LogicPortDto();
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillLogicPortDtoType", logicPortDto1, initNetworkCfg1);
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillLogicPortDtoType", logicPortDto2, initNetworkCfg2);
        Assert.assertEquals(logicPortDto1.getHomePortType(), HomePortType.VLAN);
    }

    @Test
    public void test_fill_ethernet_port() {
        List<EthPort> data = new ArrayList<>();
        EthPort ethPort = new EthPort();
        ethPort.setLocation("CTE0.IOM.H1.P0");
        ethPort.setOwnIngController("0A");
        data.add(ethPort);
        InitNetworkCfg initNetworkCfg = new InitNetworkCfg();
        initNetworkCfg.setEthernetPort("CTE0.IOM.H1.P0");
        initNetworkCfg.setController("0A");
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X8000);
        LogicPortDto logicPortDto = new LogicPortDto();
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillEthernetPort", data, logicPortDto, initNetworkCfg);
        Assert.assertEquals(logicPortDto.getHomePortName(), "CTE0.IOM.H1.P0");
    }

    @Test
    public void test_fill_bonding_port_should_throw_exception_when_empty_controller() throws Exception {
        InitNetworkCfg initNetworkCfg = new InitNetworkCfg();
        NetworkInfo networkInfo = new NetworkInfo();
        networkInfo.setMtu("1500");
        initNetworkCfg.setEthernetPort("CTE0.IOM.H1.P0,CTE0.IOM.H1.P1");
        initNetworkCfg.setNetworkInfo(networkInfo);
        LogicPortDto logicPortDto = new LogicPortDto();
        Assert.assertThrows(LegoCheckedException.class,
                () -> ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility, "fillBondingPort",
                        new ArrayList<>(), logicPortDto, initNetworkCfg));
    }

    @Test
    public void test_fill_vlan_port() {
        InitNetworkCfg initNetworkCfg = new InitNetworkCfg();
        List<EthPort> data = new ArrayList<>();
        EthPort ethPort = new EthPort();
        ethPort.setLocation("CTE0.IOM.H1.P0");
        ethPort.setOwnIngController("0A");
        data.add(ethPort);
        List<String> ports = new ArrayList<>();
        ports.add("CTE0.IOM.H1.P0");
        NetworkInfo networkInfo = new NetworkInfo();
        networkInfo.setMtu("1500");
        initNetworkCfg.setController("0A");
        initNetworkCfg.setNetworkInfo(networkInfo);
        initNetworkCfg.setEthernetPort("CTE0.IOM.H1.P0");
        initNetworkCfg.setVlanID("2500");
        initNetworkCfg.setPortType(PortType.ETHERNET.getPortType());
        LogicPortDto logicPortDto = new LogicPortDto();
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillVlanPort", data, logicPortDto, initNetworkCfg);
        Assert.assertEquals(logicPortDto.getVlan().getPortNameList().get(0), "CTE0.IOM.H1.P0");
        Assert.assertEquals(logicPortDto.getVlan().getMtu(), "1500");
    }

    @Test
    public void test_fille_logic_port_role() {
        InitNetworkCfg initNetworkCfg1 = new InitNetworkCfg();
        InitNetworkCfg initNetworkCfg2 = new InitNetworkCfg();
        InitNetworkCfg initNetworkCfg3 = new InitNetworkCfg();
        initNetworkCfg1.setServicePortType(NetPlaneType.BACKUP.getType());
        initNetworkCfg1.setNetworkInfo(new NetworkInfo());
        initNetworkCfg2.setServicePortType(NetPlaneType.COPY.getType());
        initNetworkCfg2.setNetworkInfo(new NetworkInfo());
        initNetworkCfg3.setServicePortType(NetPlaneType.ARCHIVE.getType());
        initNetworkCfg3.setNetworkInfo(new NetworkInfo());
        LogicPortDto logicPortDto1 = new LogicPortDto();
        LogicPortDto logicPortDto2 = new LogicPortDto();
        LogicPortDto logicPortDto3 = new LogicPortDto();
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillLogicPortDtoRole", logicPortDto1, initNetworkCfg1);
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillLogicPortDtoRole", logicPortDto2, initNetworkCfg2);
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillLogicPortDtoRole", logicPortDto3, initNetworkCfg3);
        Assert.assertEquals(logicPortDto1.getRole(), PortRole.SERVICE);
        Assert.assertEquals(logicPortDto2.getRole(), PortRole.TRANSLATE);
        Assert.assertEquals(logicPortDto3.getRole(), PortRole.ARCHIVE);
    }

    @Test
    public void test_fill_logic_port_contronller() {
        InitNetworkCfg initNetworkCfg = new InitNetworkCfg();
        initNetworkCfg.setController("0A");
        initNetworkCfg.setNetworkInfo(new NetworkInfo());
        LogicPortDto logicPortDto = new LogicPortDto();
        ReflectionTestUtils.invokeMethod(initializeNetworkBodyXlsAbility,
                "fillLogicPortDtoController", logicPortDto, initNetworkCfg);
        Assert.assertEquals(logicPortDto.getHomeControllerId(), "0A");
        Assert.assertEquals(logicPortDto.getCurrentControllerId(), "0A");
    }

    /**
     * 用例场景：当使用LLD方式初始化时，检查绑定端口的名字是否合法
     * 前置条件：传入的参数中包含绑定端口名称
     * 检查点：传入的是正常的绑定端口名称，检查成功
     */
    @Test
    public void test_checkAndReturnInitXls_success() {
        PowerMockito.when(deployTypeService.isSupportInitByLLD()).thenReturn(true);

        List<LogicPortAddRequest> logicPortAddRequestList = new ArrayList<>();
        LogicPortAddRequest logicPortAddRequest = new LogicPortAddRequest();
        logicPortAddRequest.setLogicalType("2");
        logicPortAddRequest.setIpv4Addr("8.40.102.105");
        logicPortAddRequestList.add(logicPortAddRequest);
        PowerMockito.when(initializePortService.getLogicPort()).thenReturn(logicPortAddRequestList);

        PowerMockito.when(initializePortService.getDeviceId()).thenReturn("2102353GTH10L8000006");

        PowerMockito.when(initializePortService.getUsername()).thenReturn("admin");

        DeviceManagerResponse<List<EthPort>> logicPortAddRequestResponse = new DeviceManagerResponse<>();
        List<EthPort> ethPortList = new ArrayList<>();
        EthPort ethPort1 = new EthPort();
        EthPort ethPort2 = new EthPort();
        EthPort ethPort3 = new EthPort();
        EthPort ethPort4 = new EthPort();
        ethPort1.setLocation("CTE0.A.IOM0.P0");
        ethPort2.setLocation("CTE0.A.IOM0.P1");
        ethPort3.setLocation("CTE0.A.IOM0.P2");
        ethPort4.setLocation("CTE0.A.IOM0.P3");
        ethPortList.add(ethPort1);
        ethPortList.add(ethPort2);
        ethPortList.add(ethPort3);
        ethPortList.add(ethPort4);

        EthPort ethPort5 = new EthPort();
        EthPort ethPort6 = new EthPort();
        EthPort ethPort7 = new EthPort();
        EthPort ethPort8 = new EthPort();
        ethPort5.setLocation("CTE0.B.IOM0.P0");
        ethPort6.setLocation("CTE0.B.IOM0.P1");
        ethPort7.setLocation("CTE0.B.IOM0.P2");
        ethPort8.setLocation("CTE0.B.IOM0.P3");
        ethPortList.add(ethPort5);
        ethPortList.add(ethPort6);
        ethPortList.add(ethPort7);
        ethPortList.add(ethPort8);
        logicPortAddRequestResponse.setData(ethPortList);
        PowerMockito.when(netWorkPortService.queryEthPorts(any(), any())).thenReturn(logicPortAddRequestResponse);

        DeviceInfo deviceInfo = PowerMockito.mock(DeviceInfo.class);
        PowerMockito.when(systemService.getDeviceInfo()).thenReturn(deviceInfo);

        PowerMockito.when(deviceInfo.getEsn()).thenReturn("2102353GTH10L8000006");

        DeviceManagerResponse<List<BondPortRes>> bondPortResResponse = new DeviceManagerResponse<>();
        PowerMockito.when(netWorkPortService.getBondPort("2102353GTH10L8000006", "admin"))
            .thenReturn(bondPortResResponse);

        MultipartFile multipartFile = ResourceHelper.createMultipartFile(getClass(), "LLD_X_test.xls");
        InitNetworkBody initNetworkBody = initializeNetworkBodyXlsAbility.checkAndReturnInitXls(multipartFile);
        Assert.assertEquals(2, initNetworkBody.getBackupNetworkConfig().getLogicPorts().size());
    }

    /**
     * 用例场景：当使用LLD方式初始化时，检查绑定端口的名字是否合法
     * 前置条件：传入的参数中已在底座自己包含相同的绑定端口名称
     * 检查点：传入的是正常的绑定端口名称，检查失败
     */
    @Test
    public void test_checkBondLogicPortName_fail() {
        List<LogicPortDto> bondLogicPorts = new ArrayList<>();

        // 异常情况1 长度大于31
        LogicPortDto logicPortDto1 = new LogicPortDto();
        BondPortPo bondPortPo1 = new BondPortPo();
        bondPortPo1.setName("张珊珊张珊珊张珊珊张");
        logicPortDto1.setBondPort(bondPortPo1);

        bondLogicPorts.add(logicPortDto1);
        PowerMockito.when(clusterBasicService.getCurrentClusterEsn()).thenReturn("123");

        DeviceManagerResponse<List<BondPortRes>> bondPortResResponse = new DeviceManagerResponse<>();
        List<BondPortRes> bondPortResList = new ArrayList<>();
        BondPortRes bondPortRes1 = new BondPortRes();
        bondPortRes1.setName("张珊珊张珊珊张珊珊张");
        BondPortRes bondPortRes2 = new BondPortRes();
        bondPortRes2.setName(".....");
        BondPortRes bondPortRes3 = new BondPortRes();
        bondPortRes3.setName("__....张珊珊");
        bondPortResList.add(bondPortRes1);
        bondPortResList.add(bondPortRes2);
        bondPortResList.add(bondPortRes3);
        bondPortResResponse.setData(bondPortResList);
        PowerMockito.when(netWorkPortService.getBondPort("123", "admin")).thenReturn(bondPortResResponse);

        Assert.assertThrows(LegoCheckedException.class, () -> initializeNetworkBodyXlsAbility.checkBondLogicPortName(bondLogicPorts));
    }

    /**
     * 绑定端口名称是正常输入的情况
     *
     * @return 绑定端口名称是正常输入的
     */
    public List<LogicPortDto> prepareNormalBondingLogicPortDtos() {
        List<LogicPortDto> bondLogicPorts = new ArrayList<>();
        LogicPortDto bondLogicPort1 = new LogicPortDto();
        bondLogicPort1.setHomePortType(HomePortType.BINDING);
        BondPortPo bondPortPo1 = new BondPortPo();
        bondPortPo1.setName("zss_test1");
        bondLogicPort1.setBondPort(bondPortPo1);
        bondLogicPorts.add(bondLogicPort1);
        return bondLogicPorts;
    }

    /**
     * 绑定端口名称是异常输入的情况
     *
     * @return 绑定端口名称是异常输入的
     */
    public List<LogicPortDto> prepareAbnormalBondingLogicPortDtos() {
        List<LogicPortDto> abnormalBondLogicPorts = new ArrayList<>();
        LogicPortDto bondLogicPort1 = new LogicPortDto();
        bondLogicPort1.setHomePortType(HomePortType.BINDING);
        BondPortPo bondPortPo1 = new BondPortPo();
        bondPortPo1.setName("?@!");
        bondLogicPort1.setBondPort(bondPortPo1);
        abnormalBondLogicPorts.add(bondLogicPort1);
        return abnormalBondLogicPorts;
    }

    public List<LogicPortDto> prepareLogicPortDto() {
        List<LogicPortDto> logicPortDtos = new ArrayList<>();
        LogicPortDto logicPortDto1 = new LogicPortDto();
        LogicPortDto logicPortDto2 = new LogicPortDto();
        LogicPortDto logicPortDto3 = new LogicPortDto();
        LogicPortDto logicPortDto4 = new LogicPortDto();
        logicPortDto1.setCurrentControllerId("0A");
        logicPortDto1.setRole(PortRole.SERVICE);
        logicPortDto2.setCurrentControllerId("0B");
        logicPortDto2.setRole(PortRole.ARCHIVE);
        logicPortDto3.setCurrentControllerId("0C");
        logicPortDto3.setRole(PortRole.SERVICE);
        logicPortDto4.setCurrentControllerId("0D");
        logicPortDto4.setRole(PortRole.SERVICE);
        logicPortDtos.add(logicPortDto1);
        logicPortDtos.add(logicPortDto2);
        logicPortDtos.add(logicPortDto3);
        logicPortDtos.add(logicPortDto4);
        return logicPortDtos;
    }
}