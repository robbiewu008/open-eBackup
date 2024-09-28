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

import openbackup.system.base.common.utils.network.AddressUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.common.validator.constants.ValidateGroups;
import openbackup.system.base.sdk.cluster.netplane.validateprovider.NetPlaneInfoReqCroupProvider;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.group.GroupSequenceProvider;

import java.util.ArrayList;
import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 网络平面信息
 *
 */
@Getter
@Setter
@GroupSequenceProvider(NetPlaneInfoReqCroupProvider.class)
public class NetPlaneInfoReq {
    /**
     * ip类型
     */
    @NotBlank
    // 0:ipv4 1:ipv6
    @Pattern(regexp = "[01]", message = "value invalid")
    private String ipType;

    /**
     * 掩码
     */
    @NotBlank
    @Pattern(regexp = RegexpConstants.IPV4_SUB_NETMASK, message = "value invalid",
        groups = {ValidateGroups.IPv4Group.class})
    @Pattern(regexp = RegexpConstants.IPV6_SUB_NETMASK, message = "value invalid",
        groups = {ValidateGroups.IPv6Group.class})
    private String mask;

    /**
     * 网关
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "value invalid",
        groups = {ValidateGroups.IPv4Group.class})
    @Pattern(regexp = AddressUtil.IPV6REG, message = "value invalid", groups = {ValidateGroups.IPv6Group.class})
    private String gateway;

    /**
     * 基础设施IP
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "value invalid",
        groups = {ValidateGroups.IPv4Group.class})
    @Pattern(regexp = AddressUtil.IPV6REG, message = "value invalid", groups = {ValidateGroups.IPv6Group.class})
    private String infraIp;

    /**
     * gaussdb IP
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "value invalid",
        groups = {ValidateGroups.IPv4Group.class})
    @Pattern(regexp = AddressUtil.IPV6REG, message = "value invalid", groups = {ValidateGroups.IPv6Group.class})
    private String gaussIp;

    /**
     * sftp IP
     */
    private String sftpIp;

    /**
     * 选择的以太网端口ID列表
     */
    @Size(min = 1, message = "At least choose one eth port")
    private List<@Pattern(regexp = "[0-9]*", message = "invalid port id") String> portList;

    @NotBlank
    // 1：太网端口、7：绑定端口:、8：vlan
    @Pattern(regexp = "[178]", message = "value invalid")
    private String homePortType;

    @NotNull(message = "value required", groups = ValidateGroups.VlanHomePort.class)
    @Valid
    private VlanPortVo vlanPort;

    @JsonProperty("shareBondPort")
    private boolean isShareBondPort = false;

    @JsonProperty("reuse")
    private boolean isReuse = false;

    /**
     * 路由列表
     */
    private List<@Valid PortRoute> portRoutes = new ArrayList<>();
}
