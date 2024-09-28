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
package openbackup.system.base.sdk.devicemanager.request;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * pacific 某节点上的业务网络配置信息
 *
 */
@Getter
@Setter
public class NodeNetworkInfoRequest {
    // 节点管理ip
    @NotNull(message = "The manage ip cannot be null")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "manage ip is invalid, not ipv4 or ipv6.")
    private String manageIp;

    // 业务网络信息
    @Size(min = 1)
    @NotNull(message = "The ipInfoList cannot be null.")
    private List<IpInfo> ipInfoList;
}
