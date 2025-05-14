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
package com.huawei.oceanprotect.system.base.initialize.network.action;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import openbackup.system.base.common.model.repository.StoragePool;
import openbackup.system.base.common.model.storage.StorageResponse;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeStoragePool;
import com.huawei.oceanprotect.system.base.initialize.bean.DeviceManagerServiceMockBean;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetPlane;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortRoute;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeUserServiceAbility;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerServiceFactory;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.config.DeviceManagerConfig;
import openbackup.system.base.common.exception.DeviceManagerException;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.controller.Controller;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.StoragePoolRestApi;
import com.huawei.oceanprotect.system.base.service.GetIpRangeService;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.support.membermodification.MemberModifier;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 测试 备份/归档 初始化/修改/扩容 流程
 *
 * @author swx1010572
 * @since 2021-01-11
 */
public class DeviceManagerHandlerTest {
    private final  DeviceManagerService service = Mockito.mock(DeviceManagerService.class);
    private final InitializeUserServiceAbility initializeUserServiceAbility = Mockito.mock(InitializeUserServiceAbility.class);
    private final DeviceManagerConfig deviceManagerConfig = Mockito.mock(DeviceManagerConfig.class);
    private final InitializeStoragePool initializeStoragePool = Mockito.mock(InitializeStoragePool.class);
    private final InitializePortService initializePortService = Mockito.mock(InitializePortService.class);
    private final InitializeNetPlane initializeNetPlane = Mockito.mock(InitializeNetPlane.class);
    private final DeviceManagerServiceFactory deviceManagerServiceFactory = Mockito.mock(DeviceManagerServiceFactory.class);
    private final InitializePortRoute initializePortRoute = Mockito.mock(InitializePortRoute.class);
    private final ProviderRegistry registry = Mockito.mock(ProviderRegistry.class);
    private final GetIpRangeService provider = Mockito.mock(GetIpRangeService.class);

    private final StoragePoolRestApi storagePoolRestApi = Mockito.mock(StoragePoolRestApi.class);

    private final DeviceManagerServiceMockBean deviceManagerServiceMockBean = new DeviceManagerServiceMockBean();

    private final DeviceManagerHandler deviceManagerHandler = new DeviceManagerHandler();

    @Before
    public void init() throws IllegalAccessException {
        MemberModifier.field(DeviceManagerHandler.class, "deviceManagerService").set(deviceManagerHandler, service);
        MemberModifier.field(DeviceManagerHandler.class, "initializeUserServiceAbility").set(deviceManagerHandler, initializeUserServiceAbility);
        MemberModifier.field(DeviceManagerHandler.class, "deviceManagerConfig").set(deviceManagerHandler, deviceManagerConfig);
        MemberModifier.field(DeviceManagerHandler.class, "initializeStoragePool").set(deviceManagerHandler, initializeStoragePool);
        MemberModifier.field(DeviceManagerHandler.class, "initializeStoragePool").set(deviceManagerHandler, initializeStoragePool);
        MemberModifier.field(DeviceManagerHandler.class, "initializePortService").set(deviceManagerHandler, initializePortService);
        MemberModifier.field(DeviceManagerHandler.class, "initializeNetPlane").set(deviceManagerHandler, initializeNetPlane);
        MemberModifier.field(DeviceManagerHandler.class, "deviceManagerServiceFactory").set(deviceManagerHandler, deviceManagerServiceFactory);
        MemberModifier.field(DeviceManagerHandler.class, "initializePortRoute").set(deviceManagerHandler, initializePortRoute);
        MemberModifier.field(DeviceManagerHandler.class, "registry").set(deviceManagerHandler, registry);
        MemberModifier.field(DeviceManagerHandler.class, "storagePoolRestApi").set(deviceManagerHandler, storagePoolRestApi);
    }

    /**
     * 用例场景：添加空闲盘到存储池成功
     * 前置条件：NA
     * 检查点：添加空闲盘到存储池成功
     */
    @Test
    public void test_add_free_disk_to_pool_success() {
        StorageResponse<List<StoragePool>> response = new StorageResponse<>();
        List<StoragePool> storagePools = new ArrayList<>();
        storagePools.add(new StoragePool());
        response.setData(storagePools);
        given(storagePoolRestApi.getStoragePools(any(), any())).willReturn(response);
        deviceManagerHandler.addFreeDiskToPool("", "");
    }

    /**
     * 用例场景：添加绑定端口成功
     * 前置条件：NA
     * 检查点：添加绑定端口成功
     */
    @Test
    public void test_add_bond_port_success() {
        deviceManagerHandler.addBondPort("", "", new ArrayList<>());
    }

    /**
     * 用例场景：获取平面网络ID字符串
     * 前置条件：NA
     * 检查点：获取平面网络ID字符串
     */
    @Test
    public void test_get_net_plane_id_infos_success() {
        List<String> netPlaneName = new ArrayList<>();
        given(initializeNetPlane.getNetPlaneIdInfos(service, netPlaneName)).willReturn("1;2;3");
        String netPlaneIdInfos = deviceManagerHandler.getNetPlaneIdInfos(service, netPlaneName);
        Assert.assertEquals("1;2;3", netPlaneIdInfos);
    }

    /**
     * 用例场景：添加逻辑端口成功
     * 前置条件：NA
     * 检查点：添加逻辑端口成功
     */
    @Test
    public void test_add_logic_port_success() {
        deviceManagerHandler.addLogicPort(new LogicPortDto());
    }

    /**
     * 用例场景：添加逻辑端口成功
     * 前置条件：NA
     * 检查点：添加逻辑端口成功
     */
    @Test
    public void test_achive_device_manager_service_success() {
        given(deviceManagerServiceFactory.getDeviceManagerService(any())).willReturn(service);
        given(deviceManagerConfig.getLink()).willReturn("127.0.0.1");
        StorageAuth storageAuth = new StorageAuth();
        storageAuth.setUsername("admin");
        storageAuth.setPassword("Huawei@123");
        deviceManagerHandler.achiveDeviceManagerService(storageAuth);
    }

    /**
     * 用例场景：获取容器pod的大小个数
     * 前置条件：NA
     * 检查点：获取容器pod的大小个数
     */
    @Test
    public void test_get_container_pod_size_success() {
        given(initializeNetPlane.getContainerPodSize(service)).willReturn(2);
        int containerPodSize = deviceManagerHandler.getContainerPodSize();
        Assert.assertEquals(2, containerPodSize);
    }

    /**
     * 用例场景：获取控制器对象信息列表
     * 前置条件：NA
     * 检查点：取控制器对象信息列表成功
     */
    @Test
    public void test_get_controllers_success() {
        given(initializeNetPlane.getController(service, 2)).willReturn(new ArrayList<>());
        List<Controller> controllers = deviceManagerHandler.getControllers(2);
        Assert.assertEquals(0, controllers.size());
    }

    /**
     * 用例场景：获取控制器对象信息列表
     * 前置条件：NA
     * 检查点：取控制器对象信息列表成功
     */
    @Test
    public void test_address_allocation_init_netPlane_front_port_success() {
        DeviceManagerResponse<List<NetPlane>> netPlanes = new DeviceManagerResponse<>();
        List<NetPlane> netPlaneList = new ArrayList<>();
        NetPlane netPlane = new NetPlane();
        netPlane.setName("backupNetPlane1");
        NetPlane netPlane1 = new NetPlane();
        netPlane1.setName("backupNetPlane");
        netPlaneList.add(netPlane);
        netPlaneList.add(netPlane1);
        netPlanes.setData(netPlaneList);
        given(service.getNetPlanes()).willReturn(netPlanes);
        DeviceManagerResponse<List<EthPort>> ethPortListResponse = new DeviceManagerResponse<>();
        ethPortListResponse.setData(new ArrayList<>());
        given(service.getEthPorts()).willReturn(ethPortListResponse);
        deviceManagerHandler.addressAllocationInitNetPlaneFrontPort("backupNetPlane", new ArrayList<>());
    }

    /**
     * 用例场景：获取控制器对象信息列表
     * 前置条件：NA
     * 检查点：取控制器对象信息列表成功
     */
    @Test
    public void should_throw_LegoCheckedException_if_create_net_plane_fail_when_address_allocation_init_netPlane_front_port() {
        DeviceManagerResponse<List<NetPlane>> netPlanes = new DeviceManagerResponse<>();
        given(service.getNetPlanes()).willReturn(netPlanes);
        DeviceManagerResponse<NetPlane> netPlaneDeviceManagerResponse = new DeviceManagerResponse<>();
        DeviceManagerResponseError deviceManagerResponseError = new DeviceManagerResponseError();
        deviceManagerResponseError.setCode(200);
        netPlaneDeviceManagerResponse.setError(deviceManagerResponseError);
        given(service.addNetPlane(any())).willReturn(netPlaneDeviceManagerResponse);
        Assert.assertThrows(LegoCheckedException.class,
            () -> deviceManagerHandler.addressAllocationInitNetPlaneFrontPort("backupNetPlane", new ArrayList<>()));
    }

    /**
     * 用例场景：获取控制器对象名称信息列表
     * 前置条件：NA
     * 检查点：取控制器对象名称信息列表成功
     */
    @Test
    public void test_get_controller_names_success() {
        given(initializeNetPlane.getContainerPodSize(service)).willReturn(2);
        List<Controller> controllers = new ArrayList<>();
        controllers.add(new Controller());
        given(initializeNetPlane.getController(service, 2)).willReturn(controllers);
        List<String> controllerList = deviceManagerHandler.getControllerNames();
        Assert.assertEquals(1, controllerList.size());
    }

    /**
     * 用例场景：完成平面网络和容器
     * 前置条件：NA
     * 检查点：完成平面网络和容器成功
     */
    @Test
    public void test_access_net_plane_and_pod_success() {
        Map<String, String> netPlaneName = new HashMap<>();
        netPlaneName.put(InitConfigConstant.BACKUP_NET_PLANE, "1");
        netPlaneName.put(InitConfigConstant.ARCHIVE_NET_PLANE, "2");
        netPlaneName.put(InitConfigConstant.COPY_NET_PLANE, "3");
        DeviceManagerResponse<List<ContainerInfo>> conApplicationList = new DeviceManagerResponse<>();
        List<ContainerInfo> list =new ArrayList<>();
        list.add(new ContainerInfo());
        conApplicationList.setData(list);
        given(service.getConApplication()).willReturn(conApplicationList);
        DeviceManagerResponse<Object> object = new DeviceManagerResponse<>();
        object.setError(new DeviceManagerResponseError());
        given(service.modifyConApplication(any())).willReturn(object);
        deviceManagerHandler.accessNetPlaneAndPod("2", "2", netPlaneName);
    }

    /**
     * 用例场景：查询容器信息失败
     * 前置条件：NA
     * 检查点：查询容器信息失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_conApplication_fail_when_access_net_plane_and_pod() {
        given(service.getConApplication()).willThrow(new DeviceManagerException(200, "", ""));
        Assert.assertThrows(LegoCheckedException.class,
            () -> deviceManagerHandler.accessNetPlaneAndPod("2", "2", new HashMap<>()));
    }
    /**
     * 用例场景：查询容器信息为空
     * 前置条件：NA
     * 检查点：查询容器信息为空
     */
    @Test
    public void should_throw_DeviceManagerException_if_get_conApplication_not_when_access_net_plane_and_pod() {
        DeviceManagerResponse<List<ContainerInfo>> conApplicationList = new DeviceManagerResponse<>();
        given(service.getConApplication()).willReturn(conApplicationList);
        Assert.assertThrows(DeviceManagerException.class,
            () -> deviceManagerHandler.accessNetPlaneAndPod("2", "2", new HashMap<>()));
    }
}
