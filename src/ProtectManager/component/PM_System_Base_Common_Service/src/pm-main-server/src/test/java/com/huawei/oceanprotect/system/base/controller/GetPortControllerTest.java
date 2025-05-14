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
package com.huawei.oceanprotect.system.base.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.times;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.dto.dorado.AllPortListResponseDto;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.model.LogicPortFilterParam;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.SupportProtocol;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.sdk.system.model.StorageAuth;
import com.huawei.oceanprotect.system.base.service.impl.strategy.deploy.InitDeployTypeStrategyContext;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;

/**
 * 测试 返回前台所有DM 端口信息
 *
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-03
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({UserUtils.class})
public class GetPortControllerTest {
    @InjectMocks
    private PortController getPortController;
    @Mock
    private InitializePortService initializePortService;

    @Mock
    private InitDeployTypeStrategyContext initDeployTypeStrategyContext;

    @Mock
    private NetWorkPortService netWorkPortService;

    @Mock
    private ClusterBasicService clusterBasicService;

    /**
     * 用例场景：查询所有端口进行
     * 前置条件：NA
     * 检查点：查看绑定,以太网,逻辑端口是否符合预期
     */
    @Test
    public void create_SystemTime_success() {
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setPassword("Huawei@123");
        storageAuth.setUsername("admin");
        AllPortListResponseDto allPortListResponseDto = new AllPortListResponseDto();
        allPortListResponseDto.setEthPortDtoList(new ArrayList<>());
        allPortListResponseDto.setLogicPortDtoList(new ArrayList<>());
        allPortListResponseDto.setBondPortList(new ArrayList<>());
        given(initializePortService.getPorts(any())).willReturn(allPortListResponseDto);
        AllPortListResponseDto allPorts = getPortController.getAllPorts(new LogicPortFilterParam());
        Assert.assertEquals(allPorts.getLogicPortDtoList().size(), 0);
        Assert.assertEquals(allPorts.getLogicPortDtoList().size(), 0);
        Assert.assertEquals(allPorts.getLogicPortDtoList().size(), 0);
    }

    /**
     * 用例场景：新增逻辑端口
     * 前置条件：NA
     * 检查点：新增逻辑端口成功
     */
    @Test
    public void should_add_logic_port_success() {
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setPassword("Huawei@123");
        storageAuth.setUsername("admin");
        LogicPortDto logicPort = new LogicPortDto();
        logicPort.setIp("192.168.111.114");
        logicPort.setName("back1");
        logicPort.setHomePortName("CTE0.A.IOM0.P0");
        logicPort.setHomePortType(HomePortType.ETHERNETPORT);
        logicPort.setMask("255.255.0.0");
        logicPort.setRole(PortRole.SERVICE);
        logicPort.setIpType("IPV4");
        logicPort.setSupportProtocol(SupportProtocol.NFS_CIFS);
        getPortController.addPorts(logicPort);
        Mockito.doNothing().when(initializePortService).addLogicPort(any());
        Mockito.verify(initializePortService, times(1)).addLogicPort(any());
    }

    /**
     * 用例场景：查询所有端口组
     * 前置条件：NA
     * 检查点：查询所有端口组成功
     */
    @Test
    public void should_get_failover_group_success() {
        PowerMockito.mockStatic(UserUtils.class);
        PowerMockito.when(UserUtils.getBusinessUsername()).thenReturn("dataprotect_admin");
        Mockito.doNothing().when(netWorkPortService).queryFailovergroup("123", "dataprotect_admin");
        getPortController.getFailoverGroup();
        Mockito.verify(netWorkPortService, times(1)).queryFailovergroup(any(), any());
    }
}
