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
package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.model.NetworkInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;

import lombok.Data;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 过去LLD配置信息中间类
 *
 */
@Data
public class ExcelNetworkConfigBean {
    /**
     * 业务端口类型
     */
    private String servicePortType;

    /**
     * 控制器
     */
    private String controller;

    /**
     * 以太网端口
     */
    private String ethernetPort;

    /**
     * 端口类型
     */
    private String portType;

    /**
     * vlan ID
     */
    private String vlanID;

    /**
     * 网络信息
     */
    private NetworkInfo networkInfo;

    /**
     * 路由
     */
    private List<PortRouteInfo> route;

    /**
     * 创建绑定端口名称
     */
    private String bondPortName;

    /**
     * 根据属性参数返回设置属性值
     *
     * @param value 属性值
     * @param filed 属性参数
     */
    public void setValues(String value, String filed) {
        switch (filed) {
            case "servicePortType":
                servicePortType = value;
                break;
            case "controller":
                controller = value;
                break;
            case "ethernetPort":
                ethernetPort = value;
                break;
            case "portType":
                portType = value;
                break;
            case "vlanID":
                vlanID = value;
                break;
            case "networkInfo":
                networkInfo = setNetworkInfo(value);
                break;
            case "route":
                route = setRoute(value);
                break;
            case "bondPortName":
                bondPortName = value;
                break;
            default:
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "no filed exist");
        }
    }

    private NetworkInfo setNetworkInfo(String value) {
        String[] values = value.split(",");
        if (values.length != 4) {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[]{"network_info_label", value}, "Illegal network info.");
        }
        NetworkInfo portNetInfo = new NetworkInfo();
        portNetInfo.setIp(values[0]);
        portNetInfo.setMask(values[1]);
        portNetInfo.setGateway(values[2]);
        portNetInfo.setMtu(values[3]);
        return portNetInfo;
    }

    private List<PortRouteInfo> setRoute(String value) {
        List<PortRouteInfo> routes = new ArrayList<>();
        if (StringUtils.isEmpty(value)) {
            return routes;
        }
        String[] values = value.split(";");
        for (String routeInfo : values) {
            String[] infos = routeInfo.split("/");
            if (infos.length != 4) {
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                        new String[]{"route_information_init_system_label", routeInfo}, "Illegal route info.");
            }
            PortRouteInfo portRoute = new PortRouteInfo();
            if (!RouteType.ROUTE_TYPES.contains(infos[0])) {
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                        new String[]{"route_type_label", infos[0]}, "Illegal route type.");
            }
            portRoute.setRouteType(RouteType.forValues(infos[0]));
            portRoute.setDestination(infos[1]);
            portRoute.setMask(infos[2]);
            portRoute.setGateway(infos[3]);
            routes.add(portRoute);
        }
        return routes;
    }
}
