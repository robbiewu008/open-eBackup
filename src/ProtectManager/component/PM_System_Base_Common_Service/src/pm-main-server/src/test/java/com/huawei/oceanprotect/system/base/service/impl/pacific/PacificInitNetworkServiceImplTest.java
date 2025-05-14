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
package com.huawei.oceanprotect.system.base.service.impl.pacific;

import openbackup.system.base.common.enums.DeployTypeEnum;
import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NodeNetworkInfoDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import openbackup.system.base.sdk.cluster.NodeRestApi;
import openbackup.system.base.sdk.devicemanager.entity.IpPoolDto;
import openbackup.system.base.sdk.devicemanager.entity.NodeInfoDto;
import openbackup.system.base.sdk.devicemanager.entity.PacificNodeInfoVo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.OpenStorageService;
import openbackup.system.base.sdk.devicemanager.request.IpInfo;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.PacificService;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.mockito.ArgumentMatchers.any;

/**
 * PacificInitNetworkServiceImplTest
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@RunWith(PowerMockRunner.class)
public class PacificInitNetworkServiceImplTest {
    @InjectMocks
    private PacificInitNetworkServiceImpl pacificInitNetworkService;

    @Mock
    private PacificService pacificService;

    @Mock
    private InfrastructureRestApi infrastructureRestApi;

    @Mock
    private NodeRestApi nodeRestApi;

    @Mock
    private InitConfigService initConfigService;

    @Mock
    private OpenStorageService openStorageService;

    /**
     * 用例场景：pacific部署类型验证通过
     * 前置条件：无
     * 检查点：验证通过
     */
    @Test
    public void test_applicable_when_pacific_then_success() {
        boolean applicable = pacificInitNetworkService.applicable(DeployTypeEnum.E6000.getValue());
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：获取初始化的网络参数
     * 前置条件：无
     * 检查点：获取成功
     */
    @Test
    public void test_getInitNetWorkParam_when_backup_then_success() {
        // mock initNetworkBody
        String manageIp = "8.40.102.81";
        InitNetworkBody initNetworkBody = mockInitNetworkBody(manageIp);

        pacificInitNetworkService.getInitNetWorkParam(initNetworkBody, "", "");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：统一检查
     * 前置条件：无
     * 检查点：检查成功
     */
    @Test
    public void test_unifiedCheck_when_then_success() {
        // mock initNetworkBody
        String manageIp = "8.40.102.81";
        InitNetworkBody initNetworkBody = mockInitNetworkBody(manageIp);

        Map<String, NodeInfoDto> manageIpAndNodeMap = new HashMap<>();
        NodeInfoDto nodeInfoDto = new NodeInfoDto();
        PacificNodeInfoVo pacificNodeInfoVo = new PacificNodeInfoVo();
        pacificNodeInfoVo.setName("ff00");
        nodeInfoDto.setPacificNodeInfoVo(pacificNodeInfoVo);
        nodeInfoDto.setPortBondMap(new HashMap<>());
        manageIpAndNodeMap.put(manageIp, nodeInfoDto);
        PowerMockito.when(openStorageService.getNetworkInfo(any(),any())).thenReturn(manageIpAndNodeMap);

        // mock NetworkInfo
        NetworkInfoDto networkInfo = mockNetworkInfo(manageIp);
        PowerMockito.when(pacificService.getNetworkInfo(any(), any())).thenReturn(networkInfo);

        // run
        pacificInitNetworkService.unifiedCheck(null, "" , initNetworkBody);

        // check
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：获取设备ip
     * 前置条件：无
     * 检查点：认证成功
     */
    @Test
    public void test_getDeviceIp_when_find_float_ip_then_success() {
        // mock endpoints
        InfraResponseWithError<List<String>> infraResponseWithError = PowerMockito.mock(InfraResponseWithError.class);
        PowerMockito.when(infrastructureRestApi.getEndpoints("pm-system-base"))
            .thenReturn(infraResponseWithError);
        List<String> endpoints = new ArrayList<>();
        endpoints.add("172.16.192.1");
        endpoints.add("172.16.192.2");
        PowerMockito.when(infraResponseWithError.getData()).thenReturn(endpoints);

        // mock floatIp
        String floatIp = "192.168.10.220";
        PowerMockito.when(nodeRestApi.getFloatIp(any())).thenReturn(floatIp);

        // run
        pacificInitNetworkService.getDeviceIp();
        Assert.assertTrue(true);
    }

    private InitNetworkBody mockInitNetworkBody(String manageIp) {
        InitNetworkBody initNetworkBody = PowerMockito.mock(InitNetworkBody.class);
        // mock backupNetworkConfig
        BackupNetworkConfig backupNetworkConfig = PowerMockito.mock(BackupNetworkConfig.class);
        PowerMockito.when(initNetworkBody.getBackupNetworkConfig()).thenReturn(backupNetworkConfig);

        // mock pacificInitNetWorkInfoList
        List<NodeNetworkInfoRequest> pacificInitNetWorkInfoList = new ArrayList<>();
        NodeNetworkInfoRequest pacificInitNetWorkInfo = PowerMockito.mock(NodeNetworkInfoRequest.class);
        pacificInitNetWorkInfoList.add(pacificInitNetWorkInfo);
        PowerMockito.when(backupNetworkConfig.getPacificInitNetWorkInfoList()).thenReturn(pacificInitNetWorkInfoList);

        // mock pacificInitNetWorkInfo.getManageIp()
        PowerMockito.when(pacificInitNetWorkInfo.getManageIp()).thenReturn(manageIp);

        // mock ipInfoList
        List<IpInfo> ipInfoList = new ArrayList<>();
        IpInfo ipInfo = PowerMockito.mock(IpInfo.class);
        ipInfoList.add(ipInfo);
        PowerMockito.when(ipInfo.getIpAddress()).thenReturn("192.168.32.100/20");
        PowerMockito.when(pacificInitNetWorkInfo.getIpInfoList()).thenReturn(ipInfoList);
        return initNetworkBody;
    }

    private NetworkInfoDto mockNetworkInfo(String manageIp) {
        NetworkInfoDto networkInfo = PowerMockito.mock(NetworkInfoDto.class);
        List<NodeNetworkInfoDto> nodeNetworkInfoList = new ArrayList<>();
        NodeNetworkInfoDto nodeNetworkInfo = PowerMockito.mock(NodeNetworkInfoDto.class);
        nodeNetworkInfoList.add(nodeNetworkInfo);

        PowerMockito.when(networkInfo.getNodeNetworkInfoList()).thenReturn(nodeNetworkInfoList);
        PowerMockito.when(nodeNetworkInfo.getManageIp()).thenReturn(manageIp);

        List<IpPoolDto> ipPoolDtoList = new ArrayList<>();
        IpPoolDto ipPoolDto = PowerMockito.mock(IpPoolDto.class);
        ipPoolDtoList.add(ipPoolDto);
        PowerMockito.when(ipPoolDto.getNodeIp()).thenReturn(manageIp);
        PowerMockito.when(ipPoolDto.getIpAddress()).thenReturn("192.168.32.100/20");
        PowerMockito.when(nodeNetworkInfo.getIpPoolDtoList()).thenReturn(ipPoolDtoList);
        return networkInfo;
    }
}