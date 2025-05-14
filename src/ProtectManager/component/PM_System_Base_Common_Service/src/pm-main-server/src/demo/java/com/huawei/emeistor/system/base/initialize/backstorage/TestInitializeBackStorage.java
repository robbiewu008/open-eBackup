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
package com.huawei.oceanprotect.system.base.initialize.backstorage;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitBackStorageAbility;
import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitFileSystemsAbility;
import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitNfsClientAbility;
import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitNfsShareAbility;
import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitStoragePoolAbility;
import com.huawei.oceanprotect.system.base.initialize.backstorage.beans.InitBackActionResult;
import com.huawei.oceanprotect.system.base.initialize.backstorage.service.InitializeControllerAddr;
import com.huawei.oceanprotect.system.base.initialize.backstorage.service.services.InitControllerAddrService;
import com.huawei.oceanprotect.system.base.initialize.network.dao.InitNetworkConfigMapper;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.accesspoint.model.InitializeResult;
import com.huawei.oceanprotect.system.base.sdk.accesspoint.model.InitializeResultDesc;
import com.huawei.oceanprotect.system.base.sdk.accesspoint.model.enums.InitializeResultCode;
import com.huawei.oceanprotect.system.base.sdk.accesspoint.restapi.InitializeRestApi;
import com.huawei.oceanprotect.system.base.sdk.accesspoint.restapi.QueryMountPathRestApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerServiceFactory;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.DeviceManagerServiceFactoryAbility;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfo;
import com.huawei.oceanprotect.system.base.sdk.infrastructure.InfrastructureRestApi;
import com.huawei.oceanprotect.system.base.sdk.infrastructure.model.InfraResponseWithError;
import com.huawei.oceanprotect.system.base.sdk.infrastructure.model.beans.NetPlaneInfo;
import com.huawei.oceanprotect.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
 * 测试设置存储池能力
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    DeviceManagerServiceFactory.class, DeviceManagerServiceFactoryAbility.class, InitializeBackStorage.class,
    InitBackStorageAbility.class, InitializeStoragePool.class, InitStoragePoolAbility.class, InitializeNfsShare.class,
    InitNfsShareAbility.class, InitializeNfsClient.class, InitNfsClientAbility.class, InitializeFileSystems.class,
    InitFileSystemsAbility.class, InitializeControllerAddr.class, InitControllerAddrService.class,
    InitStatusService.class
})
@AutoConfigureMockMvc
public class TestInitializeBackStorage {
    @Autowired
    private DeviceManagerServiceFactory deviceManagerServiceFactory;

    @Autowired
    private InitializeBackStorage initializeBackStorage;

    @MockBean
    private InitializeRestApi initializeRestApi;

    @MockBean
    private InfrastructureRestApi infrastructureRestApi;

    @MockBean
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @MockBean
    private QueryMountPathRestApi queryMountPathRestApi;

    private DeviceManagerService service;

    /**
     * 获取会话
     */
    @Before
    public void getDeviceManagerService() {
        service = deviceManagerServiceFactory.getDeviceManagerService(
            new DeviceManagerInfo("https://8.40.102.28:8088/", "admin_pm", "Huawei@123"));
    }

    /**
     * 测试初始化存储
     */
    @Test
    public void testInitializeBackStorage() {
        InitializeResult result = new InitializeResult();
        List<InitializeResultDesc> list = new ArrayList<>();
        list.add(new InitializeResultDesc(InitializeResultCode.SUCCESS, ""));
        result.setActionResults(list);

        NodePodInfo podInfo1 = new NodePodInfo();
        podInfo1.setNamespace("dpa");
        podInfo1.setPodName("protectengine-a-business-control-0");
        podInfo1.setNodeName("127.0.0.111");
        List<NetPlaneInfo> planes1 = new LinkedList<>();
        NetPlaneInfo planeInfo11 = new NetPlaneInfo("172.17.128.2", "nas.container.kubernetes.io/ip_address");
        NetPlaneInfo planeInfo12 = new NetPlaneInfo("127.0.0.1", "nas.storage.kubernetes.io/ip_address");
        planes1.add(planeInfo11);
        planes1.add(planeInfo12);
        podInfo1.setNetPlaneInfos(planes1);

        NodePodInfo podInfo2 = new NodePodInfo();
        podInfo2.setNamespace("dpa");
        podInfo2.setPodName("protectengine-a-business-control-1");
        podInfo2.setNodeName("127.0.0.112");
        List<NetPlaneInfo> planes2 = new LinkedList<>();
        NetPlaneInfo planeInfo21 = new NetPlaneInfo("172.17.128.134", "nas.container.kubernetes.io/ip_address");
        NetPlaneInfo planeInfo22 = new NetPlaneInfo("127.0.0.1", "nas.storage.kubernetes.io/ip_address");
        planes2.add(planeInfo21);
        planes2.add(planeInfo22);
        podInfo2.setNetPlaneInfos(planes2);

        List<NodePodInfo> pods = new LinkedList<>();
        pods.add(podInfo1);
        pods.add(podInfo2);

        InfraResponseWithError<List<NodePodInfo>> res = new InfraResponseWithError<List<NodePodInfo>>();
        res.setData(pods);

        given(infrastructureRestApi.getCollectNetPlaneInfo(any(), any())).willReturn(res);

        given(initializeRestApi.initializeBackStorage(any())).willReturn(new InitializeResult());

        initializeBackStorage.doAction(service, new InitBackActionResult());
    }
}
