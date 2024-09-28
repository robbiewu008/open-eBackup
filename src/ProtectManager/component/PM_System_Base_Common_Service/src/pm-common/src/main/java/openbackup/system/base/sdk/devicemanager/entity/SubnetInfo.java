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
 * pacific 子网信息
 *
 */
@Getter
@Setter
public class SubnetInfo {
    @JsonProperty("dns_iface_name")
    private String dnsIfaceName;

    @JsonProperty("dns_ip")
    private String dnsIp;

    private String domain;

    private String gateway;

    private String name;

    @JsonProperty("net_version")
    private String netVersion;

    @JsonProperty("node_storage_frontend_ip")
    private String nodeStorageFrontendIp;

    @JsonProperty("standby_dns_ip")
    private String standbyDnsIp;

    @JsonProperty("vlan_id")
    private int vlanId;
}
