/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkResult;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.controller.Controller;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlaneRange;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.LogicType;

import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * 初始化动作
 *
 * @author swx1010572
 * @since 2021-01-18
 */
public interface InitializeNetPlane {
    /**
     * 执行动作
     *
     * @param service dm 对象
     * @param netPlaneAccessPortList 请求参数
     * @param netPlaneName 网络平面关联枚举类
     * @return 结束动作
     */
    InitNetworkResult doAction(DeviceManagerService service, List<String> netPlaneAccessPortList,
        String netPlaneName);

    /**
     * 获取需要的控制器对象列表
     *
     * @param service dm 对象
     * @param controllerSize 需要的控制个数
     * @return 控制器对象列表
     */
    List<Controller> getController(DeviceManagerService service, int controllerSize);

    /**
     * 执行 添加平面网段IP段
     *
     * @param service dm 对象
     * @param netPlaneIpRange 请求参数
     * @param networkType 网络类型
     * @param netPlaneName 平面网络名字
     * @param ipType IP类型
     */
    void addIpRange(DeviceManagerService service, Map<String, String> netPlaneIpRange, String networkType,
        String ipType, String netPlaneName);

    /**
     * 获取平面网络信息
     *
     * @param service dm 对象
     * @param netPlaneName 平面网络名称
     * @return 平面网络信息
     */
    NetPlane getNetPlane(DeviceManagerService service, String netPlaneName);

    /**
     * 获取平面网络信息列表
     *
     * @param service dm 对象
     * @return 平面网络信息列表
     */
    List<NetPlane> getAllNetPlane(DeviceManagerService service);

    /**
     * 获取平面网络IP段信息
     *
     * @param service dm 对象
     * @param netPlaneName 平面网络名称
     * @return 平面网络IP段列表信息
     */
    List<NetPlaneRange> getIpRange(DeviceManagerService service, String netPlaneName);

    /**
     * 获取 扩容控制器 ID集合;
     *
     * @param service dm 对象
     * @param isExpansion 是否扩容
     * @param netPlaneId 平面网络ID
     * @param controllerSize 传递的实际需要控制器数量
     * @return 扩容控制器的ID集合
     */
    Set<String> getExpansionControllerId(DeviceManagerService service, boolean isExpansion, String netPlaneId,
        int controllerSize);

    /**
     * 获取对应类型的所有端口
     *
     * @param service dm 对象
     * @param logicType 端口角色类型
     * @return 端口角色类型的端口集合
     */
    List<EthPort> getEthPortList(DeviceManagerService service, LogicType logicType);

    /**
     * 获取集合端口是否存在控制器的前端卡 LocationName;
     *
     * @param service dm 对象
     * @param collect 过滤前的端口集合
     * @param controllerId 控制器的location
     * @return Locaitonname
     */
    String getEthPortOwnIngLocation(DeviceManagerService service, List<EthPort> collect, String controllerId);

    /**
     * 获取当前容器pod的size
     *
     * @param service dm 对象
     * @return containerPodSize
     */
    int getContainerPodSize(DeviceManagerService service);

    /**
     * 根据平面网络名称列表返回平面网络ID字符串
     *
     * @param service dm 对象
     * @param netPlaneName 平面网络名称列表
     * @return 平面网络ID字符串
     */
    String getNetPlaneIdInfos(DeviceManagerService service, List<String> netPlaneName);
}
