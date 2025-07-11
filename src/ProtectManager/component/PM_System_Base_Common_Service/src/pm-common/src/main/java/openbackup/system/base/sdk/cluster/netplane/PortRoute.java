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

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.utils.network.AddressUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.common.validator.constants.ValidateGroups;
import openbackup.system.base.sdk.cluster.netplane.validateprovider.PortRouteGroupProvider;

import org.hibernate.validator.group.GroupSequenceProvider;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.Pattern;

/**
 * PortRoute
 *
 */
@Getter
@Setter
@GroupSequenceProvider(PortRouteGroupProvider.class)
public class PortRoute {
    /**
     * 路由类型
     */
    @Pattern(regexp = "[012]")
    @NotBlank
    private String routeType;

    /**
     * 目标地址
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4_WITH_DEFAULT, message = "value invalid", groups = {
            ValidateGroups.IPv4Group.class})
    @Pattern(regexp = AddressUtil.IPV6REG, message = "value invalid", groups = {ValidateGroups.IPv6Group.class})
    @Pattern(regexp = "(0\\.0\\.0\\.0)|(::)", groups = {ValidateGroups.DefaultRoute.class})
    @NotBlank
    private String destination;

    /**
     * IPv4/IPv6网关
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4_WITH_DEFAULT, message = "value invalid", groups = {
            ValidateGroups.IPv4Group.class})
    @Pattern(regexp = AddressUtil.IPV6REG, message = "value invalid", groups = {ValidateGroups.IPv6Group.class})
    @NotBlank
    private String gateway;

    /**
     * 目的掩码
     */
    @NotBlank
    @Pattern(regexp = RegexpConstants.IPV4_SUB_NETMASK, message = "value invalid", groups = {
            ValidateGroups.IPv4Group.class})
    @Pattern(regexp = RegexpConstants.IPV6_SUB_NETMASK, message = "value invalid", groups = {
            ValidateGroups.IPv6Group.class})
    @Pattern(regexp = "(0\\.0\\.0\\.0)|(0)", groups = {ValidateGroups.DefaultRoute.class})
    @Pattern(regexp = "(255\\.255\\.255\\.255)|(128)", groups = {ValidateGroups.MasterRoute.class})
    private String mask;
}
