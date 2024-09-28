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
package openbackup.system.base.sdk.cluster.netplane;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 网络平面 成员信息（来源：DM sdk）
 *
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NetworkPlane {
    /**
     * id
     */
    @JsonProperty("id")
    private String id;

    /**
     * 名称
     */
    @JsonProperty("name")
    private String name;

    /**
     * 交换机的最大传输值
     */
    @JsonProperty("mtu")
    private String mtu;

    /**
     * 虚拟局域网id
     */
    @JsonProperty("vlanid")
    private String vlanid;

    /**
     * 子网库
     */
    @JsonProperty("ipv4SubNetBase")
    private String ipv4SubNetBase;

    /**
     * 子网掩码
     */
    @JsonProperty("ipv4NetMask")
    private String ipv4NetMask;

    /**
     * ipv4的网关地址
     */
    @JsonProperty("ipv4Gateway")
    private String ipv4Gateway;

    /**
     * 子网变动范围
     */
    @JsonProperty("ipv4SubNetRange")
    private String ipv4SubNetRange;

    /**
     * ipv6子网变动范围
     */
    @JsonProperty("ipv6SubNetRange")
    private String ipv6SubNetRange;

    /**
     * ipv6前缀长度
     */
    @JsonProperty("ipv6NetMask")
    private String ipv6NetMask;

    /**
     * ipv6网关地址
     */
    @JsonProperty("ipv6GateWay")
    private String ipv6GateWay;

    /**
     * ipv6子网库
     */
    @JsonProperty("ipv6SubNetBase")
    private String ipv6SubNetBase;

    /**
     * ipv6子网库
     */
    @JsonProperty("failover")
    private String failover;
}
