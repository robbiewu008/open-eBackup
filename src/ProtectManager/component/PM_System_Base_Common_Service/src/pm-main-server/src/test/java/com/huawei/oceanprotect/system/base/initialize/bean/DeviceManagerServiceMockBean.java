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
package com.huawei.oceanprotect.system.base.initialize.bean;

import openbackup.system.base.sdk.system.model.StorageAuth;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigLanguage;
import com.huawei.oceanprotect.system.base.initialize.network.common.CopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InstallationLanguageType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPortRes;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerDynamicConfigInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.controller.Controller;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.inftmodule.InftModuleObject;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HealthStatus;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.LogicType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RunningStatus;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.ServiceModeType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.SupportProtocol;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.Type;

import java.util.ArrayList;
import java.util.List;

/**
 * 初始化需要Mock的Bean对象
 *
 */
public class DeviceManagerServiceMockBean {
    /**
     * 控制器id
     */
    static final String CID0A = "0A";

    /**
     * 控制器id
     */
    static final String CID0B = "0B";

    /**
     * 字符串0
     */
    static final String STRING0 = "0";

    public List<NetPlane> getNetPlane() {
        List<NetPlane> netPlaneList = new ArrayList<>();
        NetPlane backup = new NetPlane();
        backup.setName("backupNetPlane");
        backup.setId("1");
        backup.setIpv4SubNetRange("192.168.100.10-192.168.100.15");
        backup.setIpv4NetMask("255.255.0.0");
        backup.setIpv4SubNetBase("192.168.0.0");
        backup.setIpv6SubNetRange("2017:8:42:96:c11::1000-2017:8:42:96:c11::1005");
        backup.setIpv6NetMask("64");
        backup.setIpv6SubNetBase("2017:8:42:96::0");
        NetPlane archive = new NetPlane();
        archive.setName("archiveNetPlane");
        archive.setId("2");
        archive.setIpv4SubNetRange("8.42.100.10-8.42.100.15");
        archive.setIpv4NetMask("255.255.0.0");
        archive.setIpv4SubNetBase("8.42.0.0");
        archive.setIpv6SubNetRange("2016:8:42:96:c11::1000-2016:8:42:96:c11::1005");
        archive.setIpv6NetMask("64");
        archive.setIpv6SubNetBase("2016:8:42:96::0");
        netPlaneList.add(backup);
        netPlaneList.add(archive);
        return netPlaneList;
    }

    /**
     * 模拟返回eth信息
     *
     * @return DeviceManagerResponse
     */
    public DeviceManagerResponse<List<EthPort>> getDeviceManagerEth() {
        List<EthPort> ethPorts = new ArrayList<>();
        String parentAId = CID0A + ".0";
        EthPort ethA0P0Port = new EthPort();
        ethA0P0Port.setParentId(parentAId);
        ethA0P0Port.setName("P0");
        ethA0P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethA0P0Port);

        EthPort ethA0P1Port = new EthPort();
        ethA0P1Port.setParentId(parentAId);
        ethA0P1Port.setName("P1");
        ethA0P1Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethA0P1Port);

        EthPort ethA0P2Port = new EthPort();
        ethA0P2Port.setParentId(parentAId);
        ethA0P2Port.setName("P2");
        ethA0P2Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethA0P2Port);

        EthPort ethA0P3Port = new EthPort();
        ethA0P3Port.setParentId(parentAId);
        ethA0P3Port.setName("P3");
        ethA0P3Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethA0P3Port);

        parentAId = CID0A + ".1";
        EthPort ethA1P0Port = new EthPort();
        ethA1P0Port.setId("1");
        ethA1P0Port.setParentId(parentAId);
        ethA1P0Port.setName("P0");
        ethA1P0Port.setLocation("CTE0.A.IOM1.P0");
        ethA1P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethA1P0Port.setLogicType(LogicType.FRONT_END_CONTAINER_PORT);
        ethA1P0Port.setDefWorkNode("0A");
        ethPorts.add(ethA1P0Port);

        EthPort ethA1P1Port = new EthPort();
        ethA1P1Port.setId("1");
        ethA1P1Port.setParentId(parentAId);
        ethA1P1Port.setName("P1");
        ethA1P1Port.setLocation("CTE0.A.IOM1.P1");
        ethA1P1Port.setRunningStatus(RunningStatus.LINKUP);
        ethA1P1Port.setLogicType(LogicType.FRONT_END_CONTAINER_PORT);
        ethA1P1Port.setDefWorkNode("0A");
        ethPorts.add(ethA1P1Port);

        EthPort ethA1P2Port = new EthPort();
        ethA1P2Port.setParentId(parentAId);
        ethA1P2Port.setName("P2");
        ethA1P2Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethA1P2Port);

        EthPort ethA1P3Port = new EthPort();
        ethA1P3Port.setParentId(parentAId);
        ethA1P3Port.setName("P3");
        ethA1P3Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethA1P2Port);

        parentAId = CID0A + ".3";
        EthPort ethA3P0Port = new EthPort();
        ethA3P0Port.setParentId(parentAId);
        ethA3P0Port.setName("P0");
        ethA3P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethA3P0Port);

        String parentBId = "0B.0";
        EthPort ethB0P0Port = new EthPort();
        ethB0P0Port.setParentId(parentBId);
        ethB0P0Port.setName("P0");
        ethB0P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethB0P0Port);

        EthPort ethB0P1Port = new EthPort();
        ethB0P1Port.setParentId(parentBId);
        ethB0P1Port.setName("P1");
        ethB0P1Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethB0P1Port);

        EthPort ethB0P2Port = new EthPort();
        ethB0P2Port.setParentId(parentBId);
        ethB0P2Port.setName("P2");
        ethB0P2Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethB0P2Port);

        EthPort ethB0P3Port = new EthPort();
        ethB0P3Port.setParentId(parentBId);
        ethB0P3Port.setName("P3");
        ethB0P3Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethB0P3Port);

        parentBId = "0B.1";
        EthPort ethB1P0Port = new EthPort();
        ethB1P0Port.setId("2");
        ethB1P0Port.setParentId(parentBId);
        ethB1P0Port.setName("P0");
        ethB1P0Port.setLocation("CTE0.B.IOM1.P0");
        ethB1P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethB1P0Port.setLogicType(LogicType.FRONT_END_CONTAINER_PORT);
        ethB1P0Port.setDefWorkNode("0B");
        ethPorts.add(ethB1P0Port);

        EthPort ethB1P1Port = new EthPort();
        ethB1P1Port.setId("2");
        ethB1P1Port.setParentId(parentBId);
        ethB1P1Port.setName("P1");
        ethB1P1Port.setLocation("CTE0.B.IOM1.P1");
        ethB1P1Port.setRunningStatus(RunningStatus.LINKUP);
        ethB1P1Port.setLogicType(LogicType.FRONT_END_CONTAINER_PORT);
        ethB1P1Port.setDefWorkNode("0B");
        ethPorts.add(ethB1P1Port);
        EthPort ethB2P2Port = new EthPort();
        ethB2P2Port.setParentId(parentBId);
        ethB2P2Port.setName("P2");
        ethB2P2Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethB2P2Port);

        EthPort ethB3P3Port = new EthPort();
        ethB3P3Port.setParentId(parentBId);
        ethB3P3Port.setName("P3");
        ethB3P3Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethB3P3Port);

        parentBId = "0B.3";
        EthPort ethB3P0Port = new EthPort();
        ethB3P0Port.setParentId(parentBId);
        ethB3P0Port.setName("P0");
        ethB3P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethPorts.add(ethB3P0Port);

        DeviceManagerResponse<List<EthPort>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponse.setData(ethPorts);
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        return deviceManagerResponse;
    }

    /**
     * 模拟返回eth信息
     *
     * @return DeviceManagerResponse
     */
    public DeviceManagerResponse<List<EthPort>> getDeviceManagerBackEndEth() {
        List<EthPort> ethPorts = new ArrayList<>();
        String parentAId = CID0A + ".3";
        EthPort ethA1P0Port = new EthPort();
        ethA1P0Port.setId("3");
        ethA1P0Port.setParentId(parentAId);
        ethA1P0Port.setName("P0");
        ethA1P0Port.setLocation("CTE0.A.IOM3.P0");
        ethA1P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethA1P0Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethA1P0Port.setDefWorkNode("0A");
        ethPorts.add(ethA1P0Port);

        EthPort ethA1P1Port = new EthPort();
        ethA1P1Port.setId("3");
        ethA1P1Port.setParentId(parentAId);
        ethA1P1Port.setName("P1");
        ethA1P1Port.setLocation("CTE0.A.IOM3.P1");
        ethA1P1Port.setRunningStatus(RunningStatus.LINKUP);
        ethA1P1Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethA1P1Port.setDefWorkNode("0A");
        ethPorts.add(ethA1P1Port);

        EthPort ethA1P2Port = new EthPort();
        ethA1P2Port.setId("3");
        ethA1P2Port.setParentId(parentAId);
        ethA1P2Port.setName("P2");
        ethA1P2Port.setLocation("CTE0.A.IOM3.P2");
        ethA1P2Port.setRunningStatus(RunningStatus.LINKUP);
        ethA1P2Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethA1P2Port.setDefWorkNode("0A");
        ethPorts.add(ethA1P2Port);

        EthPort ethA1P3Port = new EthPort();
        ethA1P3Port.setId("3");
        ethA1P3Port.setParentId(parentAId);
        ethA1P3Port.setName("P3");
        ethA1P3Port.setLocation("CTE0.A.IOM3.P3");
        ethA1P3Port.setRunningStatus(RunningStatus.LINKUP);
        ethA1P3Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethA1P3Port.setDefWorkNode("0B");
        ethPorts.add(ethA1P3Port);

        String parentBId = "0B.3";
        EthPort ethB1P0Port = new EthPort();
        ethB1P0Port.setId("3");
        ethB1P0Port.setParentId(parentBId);
        ethB1P0Port.setName("P0");
        ethB1P0Port.setLocation("CTE0.A.IOM3.P0");
        ethB1P0Port.setRunningStatus(RunningStatus.LINKUP);
        ethB1P0Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethB1P0Port.setDefWorkNode("0B");
        ethPorts.add(ethB1P0Port);

        EthPort ethB1P1Port = new EthPort();
        ethB1P1Port.setId("3");
        ethB1P1Port.setParentId(parentBId);
        ethB1P1Port.setName("P1");
        ethB1P1Port.setLocation("CTE0.A.IOM3.P1");
        ethB1P1Port.setRunningStatus(RunningStatus.LINKUP);
        ethB1P1Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethB1P1Port.setDefWorkNode("0B");
        ethPorts.add(ethB1P1Port);

        EthPort ethB1P2Port = new EthPort();
        ethB1P2Port.setId("3");
        ethB1P2Port.setParentId(parentBId);
        ethB1P2Port.setName("P2");
        ethB1P2Port.setLocation("CTE0.A.IOM3.P2");
        ethB1P2Port.setRunningStatus(RunningStatus.LINKUP);
        ethB1P2Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethB1P2Port.setDefWorkNode("0B");
        ethPorts.add(ethB1P2Port);

        EthPort ethB1P3Port = new EthPort();
        ethB1P3Port.setId("3");
        ethB1P3Port.setParentId(parentBId);
        ethB1P3Port.setName("P3");
        ethB1P3Port.setLocation("CTE0.A.IOM3.P3");
        ethB1P3Port.setRunningStatus(RunningStatus.LINKUP);
        ethB1P3Port.setLogicType(LogicType.BACK_END_CONTAINER_PORT);
        ethB1P3Port.setDefWorkNode("0B");
        ethPorts.add(ethB1P3Port);

        DeviceManagerResponse<List<EthPort>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponse.setData(ethPorts);
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        return deviceManagerResponse;
    }

    /**
     * 模拟设置成功
     *
     * @param deviceManagerResponse 将其设为成功
     */
    public void deviceManagerResponseSetErrorSuccess(DeviceManagerResponse<?> deviceManagerResponse) {
        DeviceManagerResponseError deviceManagerResponseError = new DeviceManagerResponseError();
        deviceManagerResponseError.setCode(0);
        deviceManagerResponseError.setDescription(STRING0);
        deviceManagerResponse.setError(deviceManagerResponseError);
    }

    /**
     * 模拟返回controller
     *
     * @return DeviceManagerResponse
     */
    public DeviceManagerResponse<List<Controller>> getDeviceManagerResponseContorller() {
        Controller controller1 = new Controller();
        controller1.setDescription("Kunpeng920 96 Cores 2.6GHz");
        controller1.setType(Type.CONTROLLER);
        controller1.setId(CID0A);
        controller1.setName("A");
        controller1.setParentType(Type.ENCLOSURE);
        controller1.setParentId(STRING0);
        controller1.setLocation("CTE0.A");
        controller1.setRunningStatus(RunningStatus.ONLINE);

        Controller controller2 = new Controller();
        controller2.setDescription("Kunpeng920 96 Cores 2.6GHz");
        controller2.setType(Type.CONTROLLER);
        controller2.setId("0B");
        controller2.setName("B");
        controller2.setParentType(Type.ENCLOSURE);
        controller2.setParentId(STRING0);
        controller2.setLocation("CTE0.B");
        controller2.setRunningStatus(RunningStatus.ONLINE);
        List<Controller> controllers = new ArrayList<>();
        controllers.add(controller1);
        controllers.add(controller2);
        DeviceManagerResponse<List<Controller>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponse.setData(controllers);
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        return deviceManagerResponse;
    }

    /**
     * 模拟返回ContainerInfo
     *
     * @return DeviceManagerResponse
     */
    public DeviceManagerResponse<List<ContainerInfo>> getDeviceManagerResponseContainerInfo() {
        DeviceManagerResponse<List<ContainerInfo>> deviceManagerResponse = new DeviceManagerResponse<>();
        List<ContainerInfo> list = new ArrayList<>();
        ContainerInfo containerInfo = new ContainerInfo();
        containerInfo.setName("dataprotect");
        containerInfo.setNamespace("dpa");
        List<ContainerDynamicConfigInfo> containerDynamicConfigInfos = new ArrayList<>();
        ContainerDynamicConfigInfo containerDynamicConfigInfo = new ContainerDynamicConfigInfo();
        containerDynamicConfigInfo.setConfigName("global.replicas");
        containerDynamicConfigInfo.setConfigValue("2");
        containerDynamicConfigInfos.add(containerDynamicConfigInfo);
        containerInfo.setDynamicConfigList(containerDynamicConfigInfos);
        list.add(containerInfo);
        deviceManagerResponse.setData(list);
        return deviceManagerResponse;
    }

    /**
     * 模拟返回绑定端口组列表
     *
     * @return List<BondPortRes>
     */
    public List<BondPortRes> getBondPortResList() {
        List<BondPortRes> bondPortResList = new ArrayList<>();
        BondPortRes ethernetPortBondPortRes = new BondPortRes();
        ethernetPortBondPortRes.setName("CTE.OA.P0");
        ethernetPortBondPortRes.setId("140203333");
        bondPortResList.add(ethernetPortBondPortRes);

        BondPortRes bindingLogicPortDto = new BondPortRes();
        bindingLogicPortDto.setName("name");
        bindingLogicPortDto.setId("155555555");
        bondPortResList.add(bindingLogicPortDto);
        return bondPortResList;
    }

    /**
     * 模拟返回接口模块信息列表
     *
     * @return List<InftModuleObject>
     */
    public List<InftModuleObject> getinftModuleObjectList() {
        List<InftModuleObject> inftModuleObjectList = new ArrayList<>();
        InftModuleObject inftModuleObjectOA = new InftModuleObject();
        inftModuleObjectOA.setServiceMode(ServiceModeType.CONTAINER_BACK_END.getMode());
        inftModuleObjectOA.setId("0A.3");
        inftModuleObjectOA.setHealthStatus(HealthStatus.NORMAL);
        inftModuleObjectList.add(inftModuleObjectOA);
        InftModuleObject inftModuleObjectOB = new InftModuleObject();
        inftModuleObjectOB.setServiceMode(ServiceModeType.CONTAINER_BACK_END.getMode());
        inftModuleObjectOB.setId("0B.3");
        inftModuleObjectOB.setHealthStatus(HealthStatus.NORMAL);
        inftModuleObjectList.add(inftModuleObjectOB);

        InftModuleObject inftModuleObjectOABackup = new InftModuleObject();
        inftModuleObjectOABackup.setServiceMode(ServiceModeType.CONTAINER_FRONT_END.getMode());
        inftModuleObjectOABackup.setId("0A.1");
        inftModuleObjectOABackup.setHealthStatus(HealthStatus.NORMAL);
        inftModuleObjectList.add(inftModuleObjectOABackup);
        InftModuleObject inftModuleObjectOBBackup = new InftModuleObject();
        inftModuleObjectOBBackup.setServiceMode(ServiceModeType.CONTAINER_FRONT_END.getMode());
        inftModuleObjectOBBackup.setId("0B.1");
        inftModuleObjectOBBackup.setHealthStatus(HealthStatus.NORMAL);
        inftModuleObjectList.add(inftModuleObjectOBBackup);
        return inftModuleObjectList;
    }

    /**
     * 模拟注入ipv4参数
     */
    public InitNetworkBody getIpv4NetworkBody() {
        // 用户认证密码
        StorageAuth auth = new StorageAuth();
        auth.setUsername("admin");
        auth.setPassword("Admin@123");

        // 国际化语言注入:
        ConfigLanguage configLanguage = new ConfigLanguage();
        configLanguage.setLanguage(InstallationLanguageType.CHINA_LANGUAGE);

        // 备份网络数据
        BackupNetworkConfig backup = new BackupNetworkConfig();
         LogicPortDto backupLogicPort0 = new LogicPortDto();
         backupLogicPort0.setName("backup0");
         backupLogicPort0.setRole(PortRole.SERVICE);
         backupLogicPort0.setCurrentControllerId(CID0A);
         backupLogicPort0.setIp("1.1.1.1");
         backupLogicPort0.setSupportProtocol(SupportProtocol.NFS_CIFS);
         LogicPortDto backupLogicPort1 = new LogicPortDto();
         backupLogicPort1.setName("backup1");
         backupLogicPort1.setRole(PortRole.SERVICE);
         backupLogicPort1.setCurrentControllerId(CID0B);
         backupLogicPort1.setIp("1.1.1.2");
         backupLogicPort1.setSupportProtocol(SupportProtocol.NFS_CIFS);
         List<LogicPortDto> backupLogicPorts = new ArrayList<>();
         backupLogicPorts.add(backupLogicPort0);
         backupLogicPorts.add(backupLogicPort1);
        backup.setLogicPorts(backupLogicPorts);

        // 归档网络数据
        ArchiveNetworkConfig archive = new ArchiveNetworkConfig();
        LogicPortDto archiveLogicPort0 = new LogicPortDto();
         archiveLogicPort0.setName("5");
         archiveLogicPort0.setRole(PortRole.SERVICE);
         archiveLogicPort0.setCurrentControllerId(CID0A);
         archiveLogicPort0.setIp("1.1.1.1");
         archiveLogicPort0.setSupportProtocol(SupportProtocol.NFS_CIFS);
         LogicPortDto archiveLogicPort1 = new LogicPortDto();
         archiveLogicPort1.setName("6");
         archiveLogicPort1.setRole(PortRole.SERVICE);
         archiveLogicPort1.setCurrentControllerId(CID0B);
         archiveLogicPort1.setIp("1.1.1.2");
         archiveLogicPort1.setSupportProtocol(SupportProtocol.NFS_CIFS);
         List<LogicPortDto> archiveLogicPorts = new ArrayList<>();
         archiveLogicPorts.add(archiveLogicPort0);
         archiveLogicPorts.add(archiveLogicPort1);
         archive.setLogicPorts(archiveLogicPorts);

         // 复制网络数据
        CopyNetworkConfig copy = new CopyNetworkConfig();

         LogicPortDto copyLogicPort1 = new LogicPortDto();
         copyLogicPort1.setName("3");
         copyLogicPort1.setRole(PortRole.TRANSLATE);
         copyLogicPort1.setCurrentControllerId(CID0A);
         copyLogicPort1.setIp("1.1.1.2");
         copyLogicPort1.setSupportProtocol(SupportProtocol.NFS_CIFS);

         LogicPortDto copyLogicPort3 = new LogicPortDto();
         copyLogicPort3.setName("4");
         copyLogicPort3.setRole(PortRole.TRANSLATE);
         copyLogicPort3.setCurrentControllerId(CID0B);
         copyLogicPort3.setIp("1.1.1.4");
         copyLogicPort3.setSupportProtocol(SupportProtocol.NFS_CIFS);
         List<LogicPortDto> copyLogicPorts = new ArrayList<>();
         copyLogicPorts.add(copyLogicPort1);
         copyLogicPorts.add(copyLogicPort3);
         copy.setLogicPorts(copyLogicPorts);

        InitNetworkBody initNetworkBody = new InitNetworkBody();
        initNetworkBody.setStorageAuth(auth);

        initNetworkBody.setBackupNetworkConfig(backup);
        initNetworkBody.setConfigLanguage(configLanguage);
        initNetworkBody.setArchiveNetworkConfig(archive);
        initNetworkBody.setCopyNetworkConfig(copy);
        return initNetworkBody;
    }

    /**
     * 模拟注入逻辑端口
     */
    public List<LogicPortDto> getLogicPortDtoList() {
        List<LogicPortDto> logicPortDtoList = new ArrayList<>();
        LogicPortDto nfsLogicPortDto = new LogicPortDto();
        nfsLogicPortDto.setRole(PortRole.SERVICE);
        nfsLogicPortDto.setSupportProtocol(SupportProtocol.NFS_CIFS);
        nfsLogicPortDto.setIpType("IPV4");
        nfsLogicPortDto.setHomePortName("CTE.OA.IOM.P0");
        nfsLogicPortDto.setHomePortType(HomePortType.ETHERNETPORT);
        logicPortDtoList.add(nfsLogicPortDto);

        LogicPortDto dataTurboLogicPortDto = new LogicPortDto();
        dataTurboLogicPortDto.setRole(PortRole.SERVICE);
        dataTurboLogicPortDto.setSupportProtocol(SupportProtocol.DATATURBO);
        dataTurboLogicPortDto.setIpType("IPV4");
        dataTurboLogicPortDto.setHomePortName("name");
        dataTurboLogicPortDto.setHomePortType(HomePortType.BINDING);
        logicPortDtoList.add(dataTurboLogicPortDto);

        LogicPortDto copyLogicPortDto = new LogicPortDto();
        copyLogicPortDto.setRole(PortRole.TRANSLATE);
        copyLogicPortDto.setIpType("IPV4");
        copyLogicPortDto.setHomePortName("CTE.OA.P0");
        copyLogicPortDto.setHomePortType(HomePortType.BINDING);
        logicPortDtoList.add(copyLogicPortDto);
        return logicPortDtoList;
    }

    /**
     * 模拟注入用户信息
     */
    public List<UserObjectResponse> getUserObjectResponse() {
        List<UserObjectResponse> list = new ArrayList<>();
        UserObjectResponse userObjectResponse = new UserObjectResponse();
        userObjectResponse.setName("datarprotect_admin");
        list.add(userObjectResponse);
        return list;
    }
}
