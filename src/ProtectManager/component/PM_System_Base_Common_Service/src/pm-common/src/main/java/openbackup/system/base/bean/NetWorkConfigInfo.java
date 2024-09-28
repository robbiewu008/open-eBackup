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
package openbackup.system.base.bean;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * 写入到configmap中的初始化网络信息映射关系类
 *
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class NetWorkConfigInfo {
    private String nodeId;

    @JsonProperty("logic_ip_list")
    private List<NetWorkLogicIp> logicIpList;

    @JsonProperty("ips_route_table")
    private List<NetWorkIpRoutesInfo> ipRouteList;
}
