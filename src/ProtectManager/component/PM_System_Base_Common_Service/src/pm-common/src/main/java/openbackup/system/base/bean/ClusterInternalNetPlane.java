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

import openbackup.system.base.sdk.cluster.netplane.BondPort;
import openbackup.system.base.sdk.cluster.netplane.PortRoute;
import openbackup.system.base.sdk.cluster.netplane.VlanPortDto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * ClusterInternalNetPlane
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-06
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class ClusterInternalNetPlane {
    private String ip;

    @JsonProperty("ip_version")
    private String ipType;

    private String mask;

    private String gateway;

    @JsonProperty("home_port_type")
    private String homePortType;

    @JsonProperty("vlan")
    private VlanPortDto vlanPort;

    @JsonProperty("bond_port")
    private BondPort bondPort;

    @JsonProperty("port_list")
    private List<String> portList;

    @JsonProperty("is_share_bond_port")
    private String isShareBondPort;

    @JsonProperty("is_reuse")
    private String isReuse;

    @JsonProperty("port_routes")
    private List<PortRoute> portRoutes;
}
