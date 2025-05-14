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

import com.huawei.oceanprotect.system.base.initialize.network.InitializePortRoute;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkResult;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitNetworkResultCode;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplaneroute.NetPlaneRoute;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import org.springframework.stereotype.Service;

import java.util.List;
import java.util.stream.Collectors;

/**
 * 路由端口 动作
 *
 * @since 2021-01-18
 */
@Slf4j
@Service
public class InitPortRouteAbility implements InitializePortRoute {
    /**
     * 路由端口动作
     *
     * @param service dm 对象
     * @param ipv4RouteCfg 请求参数
     * @param portHomeIds 逻辑端口id
     * @return 结束动作
     */
    @Override
    public InitNetworkResult doAction(DeviceManagerService service, List<Ipv4RouteInfo> ipv4RouteCfg,
        List<String> portHomeIds) {
        InitNetworkResult status = new InitNetworkResult();

        // 获取当前控制器下的 逻辑端口ids
        for (String portHomeId : portHomeIds) {
            status = createPortsRoute(status, service, portHomeId, ipv4RouteCfg, PortType.LOGICAL);
        }
        if (status.isOkay()) {
            return status;
        } else {
            log.error("create Port Route failure");
            return status.addInitBackActionResult(InitNetworkResultCode.FAILURE, "create Port Route failure");
        }
    }

    /**
     * 判断后 建路由
     *
     * @param status 当前状态
     * @param service dm 对象
     * @param portId 需要创建路由的id
     * @param ipv4RouteCfg 路由列表
     * @param portType 路由端口类型
     * @return 建立结果
     */
    private InitNetworkResult createPortsRoute(InitNetworkResult status, DeviceManagerService service, String portId,
        List<Ipv4RouteInfo> ipv4RouteCfg, PortType portType) {
        if (!status.isOkay()) {
            return status;
        }

        // 用于判断 是否拥有该逻辑端口id
        if (portId == null || "".equals(portId)) {
            log.error("not homePort id");
            return status.addInitBackActionResult(
                new InitNetworkResult(InitNetworkResultCode.FAILURE, "not homePort id"));
        }

        // 查询当前 逻辑端口的 路由 -- 做了修改或者创建的端口 路由会被删除了;
        DeviceManagerResponse<List<PortRouteInfo>> portRoutes = service.getPortRoutes(portId);
        if (!portRoutes.getError().isSuccess()) {
            return status.addInitBackActionResult(InitNetworkResultCode.FAILURE, "get port Route failure");
        }

        // 先删除new 路由中 没有的 old 路由
        for (PortRouteInfo portRouteInfo : portRoutes.getData()) {
            if (!service.deletePortRoutes(portRouteInfo).getError().isSuccess()) {
                log.error("delete oldPortRoutes failure");
                return status.addInitBackActionResult(
                    new InitNetworkResult(InitNetworkResultCode.FAILURE, "delete oldPortRoutes failure"));
            }
        }

        // 如果 需要更新的路由为null 返回;
        if (ipv4RouteCfg == null) {
            return status.addInitBackActionResult(
                new InitNetworkResult(InitNetworkResultCode.SUCCESS, "create portId portRoute success" + portId));
        }

        // 添加 old 中没有的 new 路由;
        PortRouteInfo portRouteInfo = new PortRouteInfo();
        for (Ipv4RouteInfo ipv4RouteInfo : ipv4RouteCfg) {
            portRouteInfo.setDestination(ipv4RouteInfo.getTargetAddress());
            portRouteInfo.setMask(ipv4RouteInfo.getSubNetMask());
            portRouteInfo.setGateway(ipv4RouteInfo.getGateway());
            portRouteInfo.setPortType(portType);
            portRouteInfo.setPortId(portId);
            portRouteInfo.setRouteType(RouteType.NETWORK);
            if (!service.addPortRoutes(portRouteInfo).getError().isSuccess()) {
                log.error("create portId: {} portRoute failure", portId);
                return status.addInitBackActionResult(
                    new InitNetworkResult(InitNetworkResultCode.FAILURE, "create " + portId + " portRoute failure"));
            }
        }
        return status.addInitBackActionResult(
            new InitNetworkResult(InitNetworkResultCode.SUCCESS, "create portId portRoute success" + portId));
    }

    /**
     * 删除网络平面路由
     *
     * @param service dm 对象
     * @param backupType 类型
     * @param ipv4RouteInfoList 路由列表
     * @return InitNetworkResult
     */
    @Override
    public InitNetworkResult delNetPlaneRoute(DeviceManagerService service, String backupType,
        List<Ipv4RouteInfo> ipv4RouteInfoList) {
        DeviceManagerResponse<List<NetPlaneRoute>> getNetPlaneRouteRes = service.getNetPlaneRoute();
        if (!getNetPlaneRouteRes.getError().isSuccess()) {
            log.error("get net plane route failed, ERR INFO: {}",
                getNetPlaneRouteRes.getError().getCode() + ": " + getNetPlaneRouteRes.getError().getDescription());
            return new InitNetworkResult(InitNetworkResultCode.FAILURE, "get net plane route failed");
        }
        DeviceManagerResponse<NetPlaneRoute> delNetPlaneRouteRes = null;
        List<NetPlaneRoute> netPlaneRouteList = getNetPlaneRouteRes.getData();
        if (netPlaneRouteList == null) {
            return new InitNetworkResult(InitNetworkResultCode.SUCCESS, "net plane route is null");
        }

        // 删除netPlane在ipv4Route中没有的路由
        for (NetPlaneRoute planeRoute : netPlaneRouteList) {
            if (planeRoute.getParentID().equals(backupType) && !isExistInIpv4List(planeRoute, ipv4RouteInfoList)) {
                delNetPlaneRouteRes = service.deleteNetPlaneRoute(planeRoute);
            }
            if (delNetPlaneRouteRes != null && !delNetPlaneRouteRes.getError().isSuccess()) {
                log.error("delete net plane route failed: {}",
                    planeRoute + " ERR INFO: " + delNetPlaneRouteRes.getError().getCode() + ":"
                        + delNetPlaneRouteRes.getError().getDescription());
                return new InitNetworkResult(InitNetworkResultCode.FAILURE, "delete net plane route failed");
            }
        }
        return new InitNetworkResult(InitNetworkResultCode.SUCCESS, "delete net plane route success");
    }

    /**
     * 添加网络平面路由
     *
     * @param service dm 对象
     * @param backupType 类型
     * @param ipv4RouteInfoList 路由列表
     * @return InitNetworkResult
     */
    @Override
    public InitNetworkResult addNetPlaneRoute(DeviceManagerService service, String backupType,
        List<Ipv4RouteInfo> ipv4RouteInfoList) {
        DeviceManagerResponse<List<NetPlaneRoute>> getNetPlaneRouteRes = service.getNetPlaneRoute();
        if (!getNetPlaneRouteRes.getError().isSuccess()) {
            log.error("get net plane route failed, ERR INFO: {}",
                getNetPlaneRouteRes.getError().getCode() + ": " + getNetPlaneRouteRes.getError().getDescription());
            return new InitNetworkResult(InitNetworkResultCode.FAILURE, "get net plane route failed");
        }
        DeviceManagerResponse<NetPlaneRoute> addNetPlaneRouteRes = null;
        List<NetPlaneRoute> netPlaneRouteList = getNetPlaneRouteRes.getData();

        // 添加ipv4Route在netPlane中没有的路由
        for (Ipv4RouteInfo ipv4RouteInfo : ipv4RouteInfoList) {
            if (!isExistInNPRoute(ipv4RouteInfo, netPlaneRouteList)) {
                addNetPlaneRouteRes = service.addNetPlaneRoute(
                    new NetPlaneRoute(ipv4RouteInfo.getTargetAddress(), ipv4RouteInfo.getGateway(),
                        ipv4RouteInfo.getSubNetMask(), backupType));
            }
            if (addNetPlaneRouteRes != null && !addNetPlaneRouteRes.getError().isSuccess()) {
                log.error("add net plane route failed, ERR INFO: {}",
                    addNetPlaneRouteRes.getError().getCode() + ": " + addNetPlaneRouteRes.getError().getDescription());
                return new InitNetworkResult(InitNetworkResultCode.FAILURE, "add net plane route failed");
            }
        }
        return new InitNetworkResult(InitNetworkResultCode.SUCCESS, "add net plane route success");
    }


    /**
     * 获取平面网络路由信息
     *
     * @param service dm 对象
     * @param backupType 网络平面IP类型 eg: 1:BackupNetPlane; 2: archiveNetPlane
     * @return 平面网络路由列表信息
     */
    @Override
    public List<NetPlaneRoute> getNetPlaneRoute(DeviceManagerService service, String backupType) {
        List<NetPlaneRoute> netPlaneRouteList = service.getNetPlaneRoute().getData();
        return netPlaneRouteList.stream()
            .filter(netPlaneRoute -> backupType.equals(netPlaneRoute.getParentID()))
            .collect(Collectors.toList());
    }

    private boolean isExistInIpv4List(NetPlaneRoute netPlaneRoute, List<Ipv4RouteInfo> ipv4RouteInfoList) {
        if (ipv4RouteInfoList.isEmpty()) {
            return false;
        }
        boolean isExist = false;
        for (Ipv4RouteInfo ipv4RouteInfo : ipv4RouteInfoList) {
            if (ipv4RouteInfo.getGateway().equals(netPlaneRoute.getGateWay()) && ipv4RouteInfo.getSubNetMask()
                .equals(netPlaneRoute.getMask()) && ipv4RouteInfo.getTargetAddress()
                .equals(netPlaneRoute.getDestination())) {
                isExist = true;
                break;
            }
        }
        return isExist;
    }

    private boolean isExistInNPRoute(Ipv4RouteInfo ipv4RouteInfo, List<NetPlaneRoute> netPlaneRouteList) {
        if (netPlaneRouteList.isEmpty()) {
            return false;
        }
        boolean isExist = false;
        for (NetPlaneRoute netPlaneRoute : netPlaneRouteList) {
            if (netPlaneRoute.getGateWay().equals(ipv4RouteInfo.getGateway())
                && netPlaneRoute.getMask().equals(ipv4RouteInfo.getSubNetMask())
                && netPlaneRoute.getDestination().equals(ipv4RouteInfo.getTargetAddress())) {
                isExist = true;
                break;
            }
        }
        return isExist;
    }
}
