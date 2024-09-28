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
 * pacific access zone
 *
 */
@Getter
@Setter
public class ZoneDto {
    // dns策略  1：轮询方式
    @JsonProperty("dns_strategy")
    private Integer dnsStrategy;

    // 子域名
    @JsonProperty("domain")
    private String domain;

    // 是否启用DNS服务
    @JsonProperty("enable_dns")
    private boolean isEnableDns;

    // 是否启用ip故障漂移
    @JsonProperty("enable_ip_drift")
    private boolean isEnableIpDrift;

    // 网络类型：以太
    @JsonProperty("network_type")
    private Integer networkType;

    // access zone名称
    @JsonProperty("name")
    private String name;

    // 子网名称
    @JsonProperty("subnet_name")
    private String subnetName;

    // access zone 类型
    @JsonProperty("zone_type")
    private Integer zoneType;

    // 账号id
    @JsonProperty("account_id")
    private Integer accountId;

    // 账号名称
    @JsonProperty("account_name")
    private String accountName;
}
