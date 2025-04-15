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
package openbackup.system.base.sdk.devicemanager.entity;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.TableType;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.bean.NetWorkRouteInfo;

/**
 * 绑定端口
 *
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class PortRouteInfo {
    /**
     * 目标地址
     */
    @JsonProperty("DESTINATION")
    private String destination;

    /**
     * IPv4/IPv6网关
     */
    @JsonProperty("GATEWAY")
    private String gateway;

    /**
     * 目的掩码
     */
    @JsonProperty("MASK")
    private String mask;

    /**
     * 端口id
     */
    @JsonProperty("PORTID")
    private String portId;

    /**
     * 端口名称
     */
    @JsonProperty("PORTNAME")
    private String portName;

    /**
     * 端口类型
     */
    @JsonProperty("PORTTYPE")
    private PortType portType;

    /**
     * 路由表类型
     */
    @JsonProperty("TABLETYPE")
    private TableType tableType;

    /**
     * 路由类型
     */
    @JsonProperty("TYPE")
    private RouteType routeType;

    /**
     * 转换成 NetWorkRouteInfo 对象
     *
     * @return 转换后的 NetWorkRouteInfo 对象
     */
    public NetWorkRouteInfo convertToNetWorkRouteInfo() {
        NetWorkRouteInfo info = new NetWorkRouteInfo();
        info.setMask(mask);
        info.setDestination(destination);
        info.setType(routeType.getRouteType());
        info.setGateway(gateway);
        return info;
    }
}
