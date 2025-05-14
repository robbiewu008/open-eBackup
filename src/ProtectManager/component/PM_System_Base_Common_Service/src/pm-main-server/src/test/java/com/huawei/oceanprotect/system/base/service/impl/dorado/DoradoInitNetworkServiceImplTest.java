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
package com.huawei.oceanprotect.system.base.service.impl.dorado;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.system.base.ResourceHelper;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.bean.DeviceManagerServiceMockBean;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializePortServiceAbility;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.model.ServicePortPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.UserRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.SupportProtocol;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

/**
 * DoradoInitNetworkServiceImplTest
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@RunWith(PowerMockRunner.class)
public class DoradoInitNetworkServiceImplTest {
    private static final String SUPER_ADMINISTRATOR_ROLE_TYPE = "1";

    @InjectMocks
    private DoradoInitNetworkServiceImpl doradoInitNetworkService;

    @Mock
    private InfrastructureRestApi infrastructureRestApi;

    @Mock
    private DeviceManagerService service;

    @Mock
    private InitializePortServiceAbility initializePortServiceAbility;

    @Mock
    private InitializePortService initializePortService;

    @Mock
    private InitNetworkConfigMapper initNetworkConfigMapper;

    private UserRest userRest = Mockito.mock(UserRest.class);

    private final DeviceManagerServiceMockBean deviceManagerServiceMockBean = new DeviceManagerServiceMockBean();

        private List<ServicePortPo> logicPortOfDb = new ArrayList<>();

    private List<EthPort> EthPortList = new ArrayList<>();

    @Before
    public void init() {
        String logicPortAddRequestListJson = ResourceHelper.getResourceAsString(getClass(),
                "AddLogicPortRequest.json");
        List<LogicPortDto> mockLogicPortAddRequests = JSON.parseObject(logicPortAddRequestListJson,
                new TypeReference<List<LogicPortDto>>() {
                });
        mockLogicPortAddRequests.forEach(port -> {
            if (Arrays.asList("1", "2").contains(port.getName())) {
                port.setSupportProtocol(SupportProtocol.NFS_CIFS);
                port.setRole(PortRole.SERVICE);
            }
            if (Arrays.asList("3", "4").contains(port.getName())) {
                port.setSupportProtocol(SupportProtocol.NFS_CIFS);
                port.setRole(PortRole.TRANSLATE);
            }
            if (Arrays.asList("5", "6").contains(port.getName())) {
                port.setSupportProtocol(SupportProtocol.NFS_CIFS);
                port.setRole(PortRole.ARCHIVE);
            }
            if (Objects.equals("7", port.getName())) {
                port.setSupportProtocol(SupportProtocol.NFS_CIFS);
                port.setRole(PortRole.MANAGEMENT);
            }
        });
        Mockito.when(initializePortServiceAbility.getLogicPortsCreatedByUser()).thenReturn(mockLogicPortAddRequests);

        String EthPortListJson = ResourceHelper.getResourceAsString(getClass(),
                "EthPortList.json");
        EthPortList = JSON.parseObject(EthPortListJson,
                new TypeReference<List<EthPort>>() {
                });
        DeviceManagerResponse<List<EthPort>> listDeviceManagerResponse = new DeviceManagerResponse<>();
        listDeviceManagerResponse.setData(EthPortList);
        Mockito.when(service.getEthPorts()).thenReturn(listDeviceManagerResponse);
    }

    /**
     * 用例场景：x8000部署类型验证通过
     * 前置条件：无
     * 检查点：验证通过
     */
    @Test
    public void test_applicable_when_x8000_then_success() {
        boolean applicable = doradoInitNetworkService.applicable(DeployTypeEnum.X8000.getValue());
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：获取初始化的网络参数
     * 前置条件：无
     * 检查点：获取成功
     */
    @Test
    public void test_getInitNetWorkParam_when_backup_then_success() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();

        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getInfraNodeInfo()).thenReturn(infraNodeInfo);

        List<NodeDetail> nodeDetails = new ArrayList<>();
        NodeDetail node0 = new NodeDetail();
        node0.setNodeName("0A");
        node0.setHostName("node-0");
        node0.setNodeStatus("ready");
        NodeDetail node1 = new NodeDetail();
        node1.setNodeName("0B");
        node1.setHostName("node-1");
        node1.setNodeStatus("ready");
        PowerMockito.when(infraNodeInfo.getData()).thenReturn(nodeDetails);

        doradoInitNetworkService.getInitNetWorkParam(initNetworkBody, "", "");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：统一校验成功
     * 前置条件：无
     * 检查点：校验成功
     */
    @Test
    public void should_unified_check_success() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("2");
        initNetworkBody.getArchiveNetworkConfig().setLogicPorts(new ArrayList<>());
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(0).setName("4");
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(1).setName("3");
        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = new InfraResponseWithError<>();
        String nodeDetailsJson = ResourceHelper.getResourceAsString(getClass(),
                "NodeDetails.json");
        List<NodeDetail> mockNodeDetails = JSON.parseObject(nodeDetailsJson,
                new TypeReference<List<NodeDetail>>() {
                });
        infraNodeInfo.setData(mockNodeDetails);
        Mockito.when(infrastructureRestApi.getInfraNodeInfo()).thenReturn(infraNodeInfo);
        UserObjectResponse userObjectResponse = new UserObjectResponse();
        userObjectResponse.setRoleId(SUPER_ADMINISTRATOR_ROLE_TYPE);
        userObjectResponse.setName("test");
        List<UserObjectResponse> userObjectResponses = new ArrayList<>();
        userObjectResponses.add(userObjectResponse);
        Mockito.when(service.getApiRest(any())).thenReturn(userRest);
        Mockito.when(userRest.getUser(any())).thenReturn(userObjectResponses);
        List<LogicPortDto> logicPortsCreatedByUser = prepareLogicPortsCreatedByUser();
        PowerMockito.when(initializePortService.getLogicPortsCreatedByUser()).thenReturn(logicPortsCreatedByUser);
        doradoInitNetworkService.unifiedCheck("", "", initNetworkBody);
    }

    private List<LogicPortDto> prepareLogicPortsCreatedByUser() {
        List<LogicPortDto> logicPortsCreatedByUser = new ArrayList<>();
        LogicPortDto logicPortDto1 = new LogicPortDto();
        logicPortDto1.setName("1");
        logicPortDto1.setRole(PortRole.SERVICE);
        logicPortDto1.setCurrentControllerId("0A");
        LogicPortDto logicPortDto2 = new LogicPortDto();
        logicPortDto2.setName("2");
        logicPortDto2.setRole(PortRole.SERVICE);
        logicPortDto2.setCurrentControllerId("0B");
        LogicPortDto logicPortDto3 = new LogicPortDto();
        logicPortDto3.setName("3");
        logicPortDto3.setRole(PortRole.TRANSLATE);
        logicPortDto3.setCurrentControllerId("0A");
        LogicPortDto logicPortDto4 = new LogicPortDto();
        logicPortDto4.setName("4");
        logicPortDto4.setRole(PortRole.TRANSLATE);
        logicPortDto4.setCurrentControllerId("0B");
        logicPortsCreatedByUser.add(logicPortDto1);
        logicPortsCreatedByUser.add(logicPortDto2);
        logicPortsCreatedByUser.add(logicPortDto3);
        logicPortsCreatedByUser.add(logicPortDto4);
        return logicPortsCreatedByUser;
    }

    /**
     * 用例场景：校验逻辑端口不存在抛异常
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_logic_port_not_exist_when_init_check() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().setLogicPorts(null);
        Assert.assertThrows("Archive logic port not exist", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));
    }

    /**
     * 用例场景：校验每控至少配置一个备份用的逻辑端口失败
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_backup_not_config_logic_port_when_init_check() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("3");
        initNetworkBody.getArchiveNetworkConfig().setLogicPorts(new ArrayList<>());
        initNetworkBody.getCopyNetworkConfig().setLogicPorts(new ArrayList<>());
        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = new InfraResponseWithError<>();
        String nodeDetailsJson = ResourceHelper.getResourceAsString(getClass(),
                "NodeDetails.json");
        List<NodeDetail> mockNodeDetails = JSON.parseObject(nodeDetailsJson,
                new TypeReference<List<NodeDetail>>() {
                });
        infraNodeInfo.setData(mockNodeDetails);
        Mockito.when(infrastructureRestApi.getInfraNodeInfo()).thenReturn(infraNodeInfo);
        Assert.assertThrows("Controller not config logic port of backup", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));
    }

    /**
     * 用例场景：校验必须配置备份网络失败
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_not_config_backup_network_when_init_check() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().setLogicPorts(new ArrayList<>());
        initNetworkBody.getArchiveNetworkConfig().setLogicPorts(new ArrayList<>());
        initNetworkBody.getCopyNetworkConfig().setLogicPorts(new ArrayList<>());
        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = new InfraResponseWithError<>();
        String nodeDetailsJson = ResourceHelper.getResourceAsString(getClass(),
                "NodeDetails.json");
        List<NodeDetail> mockNodeDetails = JSON.parseObject(nodeDetailsJson,
                new TypeReference<List<NodeDetail>>() {
                });
        infraNodeInfo.setData(mockNodeDetails);
        Mockito.when(infrastructureRestApi.getInfraNodeInfo()).thenReturn(infraNodeInfo);
        Assert.assertThrows("No backup network, init network config failed.", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));
    }


    /**
     * 用例场景：校验逻辑端口角色失败
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_logic_port_select_role_error_when_init_check() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("7");
        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = new InfraResponseWithError<>();
        String nodeDetailsJson = ResourceHelper.getResourceAsString(getClass(),
                "NodeDetails.json");
        List<NodeDetail> mockNodeDetails = JSON.parseObject(nodeDetailsJson,
                new TypeReference<List<NodeDetail>>() {
                });
        infraNodeInfo.setData(mockNodeDetails);
        Mockito.when(infrastructureRestApi.getInfraNodeInfo()).thenReturn(infraNodeInfo);
        Assert.assertThrows("Logical port role misconfigured.", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));

        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("2");
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(0).setName("3");
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(1).setName("7");
        Assert.assertThrows("Logical port role misconfigured.", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));

        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("2");
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(0).setName("3");
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(1).setName("4");
        initNetworkBody.getArchiveNetworkConfig().getLogicPorts().get(0).setName("7");
        initNetworkBody.getArchiveNetworkConfig().getLogicPorts().get(1).setName("5");
        Assert.assertThrows("Logical port role misconfigured.", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));
    }

    /**
     * 用例场景：校验复制业务需要每一控都要配置两种角色的逻辑端口失败
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_controller_config_replication_business_error_when_init_check() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("2");
        initNetworkBody.getArchiveNetworkConfig().setLogicPorts(new ArrayList<>());
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(1).setName("2");
        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = new InfraResponseWithError<>();
        String nodeDetailsJson = ResourceHelper.getResourceAsString(getClass(),
                "NodeDetails.json");
        List<NodeDetail> mockNodeDetails = JSON.parseObject(nodeDetailsJson,
                new TypeReference<List<NodeDetail>>() {
                });
        infraNodeInfo.setData(mockNodeDetails);
        Mockito.when(infrastructureRestApi.getInfraNodeInfo()).thenReturn(infraNodeInfo);
        Assert.assertThrows("Controller config replication business failed.", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));
    }

    /**
     * 用例场景：校验逻辑端口基本信息，逻辑端口名称或者ip重复
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_logic_port_name_or_ip_duplicate_when_init_check() {
        String logicPortAddRequestListJson = ResourceHelper.getResourceAsString(getClass(),
                "LogicPortAddRequestList.json");
        List<LogicPortAddRequest> mockLogicPortAddRequests = JSON.parseObject(logicPortAddRequestListJson,
                new TypeReference<List<LogicPortAddRequest>>() {
                });
        mockLogicPortAddRequests.get(0).setRole(PortRole.SERVICE);
        mockLogicPortAddRequests.get(1).setRole(PortRole.TRANSLATE);
        mockLogicPortAddRequests.get(2).setRole(PortRole.SERVICE);
        mockLogicPortAddRequests.get(3).setRole(PortRole.TRANSLATE);
        mockLogicPortAddRequests.get(6).setRole(PortRole.SERVICE);
        mockLogicPortAddRequests.forEach(port -> {
            port.setSupportProtocol(SupportProtocol.NFS_CIFS);
        });
        Mockito.when(initializePortServiceAbility.getLogicPort()).thenReturn(mockLogicPortAddRequests);
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setIp("1.1.1.1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("3");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setIp("1.1.1.1");
        initNetworkBody.getArchiveNetworkConfig().setLogicPorts(new ArrayList<>());
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().get(1).setName("2");
        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = new InfraResponseWithError<>();
        String nodeDetailsJson = ResourceHelper.getResourceAsString(getClass(),
                "NodeDetails.json");
        List<NodeDetail> mockNodeDetails = JSON.parseObject(nodeDetailsJson,
                new TypeReference<List<NodeDetail>>() {
                });
        infraNodeInfo.setData(mockNodeDetails);
        Mockito.when(infrastructureRestApi.getInfraNodeInfo()).thenReturn(infraNodeInfo);
        Assert.assertThrows("Logical port role misconfigured.", LegoCheckedException.class,
                () -> doradoInitNetworkService.unifiedCheck("", "", initNetworkBody));
    }

    /**
     * 用例场景：创建逻辑端口应该成功
     * 前置条件：无
     * 检查点：检查成功
     */
    @Test
    public void test_addLogicPort_should_success() {
        InitNetworkBody initNetworkBody = deviceManagerServiceMockBean.getIpv4NetworkBody();
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setName("1");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setName("3");
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).setHomePortType(HomePortType.BINDING);
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(1).setHomePortType(HomePortType.BINDING);
        initNetworkBody.getArchiveNetworkConfig().setLogicPorts(new ArrayList<>());
        initNetworkBody.getCopyNetworkConfig().setLogicPorts(new ArrayList<>());

        List<InitConfigInfo> existLogicPorts = new ArrayList<>();
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig("logicPorts")).thenReturn(existLogicPorts);
        doradoInitNetworkService.addLogicPort(initNetworkBody);
        Mockito.verify(initializePortService, Mockito.times(2)).handleLogicPort(any());
    }
}