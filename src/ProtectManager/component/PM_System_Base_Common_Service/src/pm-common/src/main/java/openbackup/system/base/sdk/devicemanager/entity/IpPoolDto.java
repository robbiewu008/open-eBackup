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

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * pacific ip信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@Getter
@Setter
public class IpPoolDto {
    // 端口名
    @JsonProperty("iface_name")
    private String ifaceName;

    // ip地址/掩码
    @JsonProperty("ip_address")
    private String ipAddress;

    // 节点前端存储网络ip
    @JsonProperty("node_ip")
    private String nodeIp;

    // 使用状态
    @JsonProperty("state")
    private Integer state;

    // access zone名称
    @JsonProperty("zone_name")
    private String zoneName;

    // access zone 的网络类型
    @JsonProperty("zone_net_type")
    private Integer zoneNetType;
}
