/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkResult;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplaneroute.NetPlaneRoute;

import java.util.List;

/**
 * 初始化动作
 *
 * @author swx1010572
 * @since 2021-01-18
 */
public interface InitializePortRoute {
    /**
     * 执行动作
     *
     * @param service dm 对象
     * @param ipv4RouteCfg 路由列表
     * @param portHomeIds 逻辑端口id
     * @return 结束动作
     */
    InitNetworkResult doAction(DeviceManagerService service, List<Ipv4RouteInfo> ipv4RouteCfg,
        List<String> portHomeIds);

    /**
     * 删除网络平面路由
     *
     * @param service dm 对象
     * @param backupType 类型
     * @param ipv4RouteInfoList 路由列表
     * @return InitNetworkResult
     */
    InitNetworkResult delNetPlaneRoute(DeviceManagerService service, String backupType,
        List<Ipv4RouteInfo> ipv4RouteInfoList);

    /**
     * 添加网络平面路由
     *
     * @param service dm 对象
     * @param backupType 类型
     * @param ipv4RouteInfoList 路由列表
     * @return InitNetworkResult
     */
    InitNetworkResult addNetPlaneRoute(DeviceManagerService service, String backupType,
        List<Ipv4RouteInfo> ipv4RouteInfoList);

    /**
     * 获取平面网络路由信息
     *
     * @param service dm 对象
     * @param backupType 网络平面IP类型 eg: 1:BackupNetPlane; 2: archiveNetPlane
     * @return 平面网络路由列表信息
     */
    List<NetPlaneRoute> getNetPlaneRoute(DeviceManagerService service, String backupType);
}
