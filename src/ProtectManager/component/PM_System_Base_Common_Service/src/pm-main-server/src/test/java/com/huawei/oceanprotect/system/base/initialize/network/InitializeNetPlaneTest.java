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

import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.initialize.bean.DeviceManagerServiceMockBean;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeNetPlaneAbility;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.enums.NetworkType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.ConApplicationDetailRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.NetPlaneRangeRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerDynamicConfigInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.controller.Controller;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlaneRange;
import openbackup.system.base.common.enums.AddressFamily;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.AssociateObjType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.LogicType;

import org.apache.commons.lang3.RandomStringUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 测试 平面网络的创建情况
 *
 */
public class InitializeNetPlaneTest {
    /**
     * 字符串0
     */
    static final String STRING0 = "0";

    /**
     * 备份网络名
     */
    private static final String BACKUP_NET_PLANE = "backupNetPlane";

    /**
     * 归档网络名
     */
    private static final String ARCHIVE_NET_PLANE = "archiveNetPlane";

    private final DeviceManagerService service = Mockito.mock(DeviceManagerService.class);

    private final NetPlaneRangeRest netPlaneRangeRest = Mockito.mock(NetPlaneRangeRest.class);
    private final ConApplicationDetailRest conApplicationDetailRest = Mockito.mock(ConApplicationDetailRest.class);

    private final InitializeNetPlane initializeNetPlane = new InitializeNetPlaneAbility();

    private final DeviceManagerServiceMockBean deviceManagerServiceMockBean = new DeviceManagerServiceMockBean();

    @Before
    public void init() {
        given(service.getNetPlanes()).willReturn(netPlanesDeviceManagerResponse());
        given(service.addPlaneAssociate(any(long.class), any())).willReturn(
            planeAssociateDeviceManagerResponse(NetPlane.class));
        given(service.getEthPortsAssociate(any(), any())).willReturn(ethAssociateDeviceManagerResponse());
        given(service.getApiRest(NetPlaneRangeRest.class)).willReturn(netPlaneRangeRest);
        given(netPlaneRangeRest.getPlaneIpRange(any(), any(), any())).willReturn(getPlaneIpRange());
    }

    private List<NetPlaneRange> getPlaneIpRange() {
        List<NetPlaneRange> list = new ArrayList<>();
        NetPlaneRange netPlaneRange = new NetPlaneRange();
        netPlaneRange.setName("backup0");
        netPlaneRange.setAddressFamily(AddressFamily.IPV4);
        netPlaneRange.setIpv4Range("192.168.50.10-192.168.50.14");
        NetPlaneRange netPlaneRange1 = new NetPlaneRange();
        netPlaneRange1.setName("backupNetPlane");
        netPlaneRange1.setAddressFamily(AddressFamily.IPV4);
        netPlaneRange1.setIpv4Range("192.168.50.5-192.168.50.8");
        list.add(netPlaneRange);
        list.add(netPlaneRange1);
        return list;
    }

    /**
     * 测试平面网络 绑定 能力 成功-001
     *
     */
    @Test
    public void test_action_success() {
        List<String> portGroup = NetPlaneAccessPortList();

        // 如果为true测试备份关联能力,如果为false测试归档关联能力
        Assert.assertTrue(initializeNetPlane.doAction(service, portGroup, "backupNetPlane").isOkay());

    }

    /**
     * 测试备份平面网络 IP段的修改能力 能力 成功 - 002
     * 用例场景：对于增加 备份平面网络的IP段 方法 进行测试
     * 前置条件：需要存在ipRange Map对象
     * 检查点：是否报错
     */
    @Test
    public void test_add_ipv4_backup_ip_range_success() {
        Map<String, String> ipRange = new HashMap<>();
        ipRange.put("backup0", "192.168.50.10-192.168.50.14");
        ipRange.put("backup1", "192.168.50.19-192.168.50.24");
        // 如果为true测试备份关联能力,如果为 false测试归档关联能力
        initializeNetPlane.addIpRange(service, ipRange, NetworkType.BACKUP.getType(), InitConfigConstant.IPV4_TYPE_FLAG,
            "backupNetPlane");
    }

    /**
     * 用例场景：对于获取 平面网络信息 方法 进行测试
     * 前置条件：需要存在传入名称的平面网络
     * 检查点：是否报错
     */
    @Test
    public void get_netplane_success() {
        List<NetPlane> netPlaneList = new ArrayList<>();
        NetPlane netPlane = new NetPlane();
        netPlane.setName("backupNetPlane");
        netPlane.setIpv4SubNetRange("192.168.50.10-192.168.50.14");
        netPlaneList.add(netPlane);
        DeviceManagerResponse<List<NetPlane>> objectDeviceManagerResponse = new DeviceManagerResponse<>();
        objectDeviceManagerResponse.setData(netPlaneList);
        given(service.getNetPlanes()).willReturn(objectDeviceManagerResponse);
        NetPlane backupNetPlane = initializeNetPlane.getNetPlane(service, "backupNetPlane");
        Assert.assertEquals("backupNetPlane", backupNetPlane.getName());
        Assert.assertEquals("192.168.50.10-192.168.50.14",backupNetPlane.getIpv4SubNetRange());
    }


    /**
     * 用例场景：对于备份平面网络 IP段 获取方法 进行测试
     * 前置条件：需要存在传入名称的平面网络
     * 检查点：1.返回参数是否只有一个IP段, 2.返回参数是否名字相同 3. IP段信息是否一致
     */
    @Test
    public void get_ipv4_backup_ip_range_success() {
        List<NetPlaneRange> backupNetPlaneList = initializeNetPlane.getIpRange(service, "backupNetPlane");
        Assert.assertEquals(2, backupNetPlaneList.size());
        Assert.assertEquals("backup0", backupNetPlaneList.get(0).getName());
        Assert.assertEquals("192.168.50.10-192.168.50.14", backupNetPlaneList.get(0).getIpv4Range());
    }

    /**
     * 用例场景：获取需要添加控制器对象成功
     * 前置条件：获取前两个控制器对象
     * 检查点：能够获取成功
     */
    @Test
    public void get_controller_size_list_success() {
        given(service.getControllers()).willReturn(deviceManagerServiceMockBean.getDeviceManagerResponseContorller());
        List<Controller> controller = initializeNetPlane.getController(service, 2);
        Assert.assertEquals(2, controller.size());
    }

    /**
     * 用例场景：获取需要添加控制器对象失败
     * 前置条件：获取前面一个控制器
     * 检查点：当控制器对象不足2时无法获取
     */
    @Test
    public void should_throw_Exception_if_get_controller_size_list_when_get_controller() {
        given(service.getControllers()).willReturn(deviceManagerServiceMockBean.getDeviceManagerResponseContorller());
        Assert.assertThrows("controllerSizeList is no exist", LegoCheckedException.class,
            () ->initializeNetPlane.getController(service, 1));
    }

    /**
     * 添加 关联端口 的参数
     *
     * @return 参数
     */
    private List<String> NetPlaneAccessPortList() {
        List<String> portGroup = new ArrayList<>();
        portGroup.add("A1P1");
        portGroup.add("A1P0");
        return portGroup;
    }

    /**
     * 模拟返回NetPlanes
     *
     * @return DeviceManagerResponse
     */
    private DeviceManagerResponse<List<NetPlane>> netPlanesDeviceManagerResponse() {
        NetPlane backupNetPlane1 = new NetPlane();
        backupNetPlane1.setName(BACKUP_NET_PLANE);
        backupNetPlane1.setId("1");
        NetPlane backupNetPlane2 = new NetPlane();
        backupNetPlane2.setName(BACKUP_NET_PLANE);
        backupNetPlane2.setId("2");
        NetPlane archiveNetPlane1 = new NetPlane();
        archiveNetPlane1.setName(ARCHIVE_NET_PLANE);
        archiveNetPlane1.setId("3");
        NetPlane archiveNetPlane2 = new NetPlane();
        archiveNetPlane2.setName(ARCHIVE_NET_PLANE);
        archiveNetPlane2.setId("4");
        ArrayList<NetPlane> netPlanes = new ArrayList<>();
        netPlanes.add(backupNetPlane1);
        netPlanes.add(backupNetPlane2);
        netPlanes.add(archiveNetPlane1);
        netPlanes.add(archiveNetPlane2);
        DeviceManagerResponse<List<NetPlane>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponse.setData(netPlanes);
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        return deviceManagerResponse;
    }

    /**
     * 模拟添加正确错误码
     */
    private void deviceManagerResponseSetErrorSuccess(DeviceManagerResponse<?> deviceManagerResponse) {
        DeviceManagerResponseError deviceManagerResponseError = new DeviceManagerResponseError();
        deviceManagerResponseError.setCode(0);
        deviceManagerResponseError.setDescription(STRING0);
        deviceManagerResponse.setError(deviceManagerResponseError);
    }

    /**
     * 模拟返回 关联正确性
     *
     * @return DeviceManagerResponse 关联正确性
     */
    private <T> DeviceManagerResponse<T> planeAssociateDeviceManagerResponse(Class<T> t) {
        DeviceManagerResponse<T> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        return deviceManagerResponse;
    }

    /**
     * 模拟返回EthPorts
     *
     * @return DeviceManagerResponse EthPortsDeviceManagerResponse
     */
    private DeviceManagerResponse<List<EthPort>> ethAssociateDeviceManagerResponse() {
        DeviceManagerResponse<List<EthPort>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponseSetErrorSuccess(deviceManagerResponse);
        List<EthPort> ethPortList = new ArrayList<>();
        EthPort ethPort = new EthPort();
        ethPort.setId(RandomStringUtils.randomNumeric(10));
        ethPortList.add(ethPort);
        ethPort.setId(RandomStringUtils.randomNumeric(10));
        ethPortList.add(ethPort);
        deviceManagerResponse.setData(ethPortList);
        return deviceManagerResponse;
    }

    /**
     * 用例场景：获取 扩容控制器 ID集合;
     * 前置条件：NA
     * 检查点：获取 扩容控制器 ID集合成功
     */
    @Test
    public void get_expansion_controller_id_success() {
        given(service.getControllers()).willReturn(deviceManagerServiceMockBean.getDeviceManagerResponseContorller());
        DeviceManagerResponse<List<EthPort>> ethPort = new DeviceManagerResponse<>();
        ethPort.setData(new ArrayList<>());
        given(service.getEthPortsAssociate(AssociateObjType.NET_PLANE, "1")).willReturn(ethPort);
        Assert.assertEquals(2, initializeNetPlane.getExpansionControllerId(service, true, "1", 2).size());
        List<EthPort> list = new ArrayList<>();
        EthPort ethPort1 = new EthPort();
        ethPort1.setDefWorkNode("0A");
        list.add(ethPort1);
        ethPort.setData(list);
        given(service.getEthPortsAssociate(AssociateObjType.NET_PLANE, "1")).willReturn(ethPort);
        Assert.assertEquals(1, initializeNetPlane.getExpansionControllerId(service, true, "1", 2).size());
    }

    /**
     * 用例场景：获取对应类型的所有端口
     * 前置条件：NA
     * 检查点：获取对应类型的所有端口成功
     */
    @Test
    public void get_eth_port_list_success() {
        DeviceManagerResponse<List<EthPort>> ethPort = new DeviceManagerResponse<>();
        ethPort.setData(new ArrayList<>());
        given(service.getEthPorts()).willReturn(ethPort);
        Assert.assertEquals(0, initializeNetPlane.getEthPortList(service, LogicType.FRONT_END_CONTAINER_PORT).size());
    }

    /**
     * 用例场景：获取集合端口是否存在控制器的前端卡 LocationName;
     * 前置条件：NA
     * 检查点：获取集合端口是否存在控制器的前端卡 LocationName成功
     */
    @Test
    public void get_eth_port_own_ing_location_success() {
        List<EthPort> list = new ArrayList<>();
        EthPort ethPort = new EthPort();
        ethPort.setDefWorkNode("0A");
        ethPort.setLocation("0A.IOM1.P0");
        ethPort.setName("0A");
        list.add(ethPort);
        Assert.assertEquals(".IOM1.P0", initializeNetPlane.getEthPortOwnIngLocation(service, list, "0A"));
    }

    /**
     * 用例场景：获取集合端口是否存在控制器的前端卡 LocationName;
     * 前置条件：NA
     * 检查点：获取集合端口是否存在控制器的前端卡 LocationName失败
     */
    @Test
    public void should_throw_Exception_if_get_eth_port_not_exist_list_when_get_eth_port_own_ing_location() {
        List<EthPort> list = new ArrayList<>();
        EthPort ethPort = new EthPort();
        ethPort.setDefWorkNode("0A");
        ethPort.setLocation("0A.IOM1.P0");
        ethPort.setName("0A");
        list.add(ethPort);
        Assert.assertThrows("no find exit [{OA}] ethPort", LegoCheckedException.class,
            () -> initializeNetPlane.getEthPortOwnIngLocation(service, list, "OA"));
    }

    /**
     * 用例场景：根据平面网络名称列表返回平面网络ID字符串
     * 前置条件：NA
     * 检查点：根据平面网络名称列表返回平面网络ID字符串成功
     */
    @Test
    public void get_net_plane_id_infos_success() {
        List<String> netPlaneName = new ArrayList<>();
        netPlaneName.add("backupNetPlane");
        netPlaneName.add("archiveNetPlane");
        Assert.assertEquals("1;2;3;4", initializeNetPlane.getNetPlaneIdInfos(service, netPlaneName));
    }

    /**
     * 用例场景：获取当前容器pod的size
     * 前置条件：NA
     * 检查点：获取当前容器pod的size成功
     */
    @Test
    public void get_container_pod_size_success() {
        DeviceManagerResponse<List<ContainerInfo>> deviceManagerResponse = new DeviceManagerResponse<>();
        List<ContainerInfo> containerInfos = new ArrayList<>();
        ContainerInfo containerInfo = new ContainerInfo();
        containerInfo.setName("dataprotect");
        containerInfo.setNamespace("dpa");
        List<ContainerDynamicConfigInfo> dynamicConfigList = new ArrayList<>();
        ContainerDynamicConfigInfo containerDynamicConfigInfo = new ContainerDynamicConfigInfo();
        containerDynamicConfigInfo.setConfigName("global.replicas");
        containerDynamicConfigInfo.setConfigValue("2");
        dynamicConfigList.add(containerDynamicConfigInfo);
        containerInfo.setDynamicConfigList(dynamicConfigList);
        containerInfos.add(containerInfo);
        deviceManagerResponse.setData(containerInfos);
        given(service.getConApplication()).willReturn(deviceManagerResponse);
        given(service.getApiRest(ConApplicationDetailRest.class)).willReturn(conApplicationDetailRest);
        given(conApplicationDetailRest.getConApplication("dataprotect", "dpa")).willReturn(containerInfo);
        Assert.assertEquals(2, initializeNetPlane.getContainerPodSize(service));
    }
}
