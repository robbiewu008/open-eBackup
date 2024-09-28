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
package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Storage front service ips info
 *
 */

@Data
public class StorageFrontServiceInfo {
    @JsonProperty("name")
    @JsonAlias("NAME")
    private String name;

    @JsonProperty("currentControllerId")
    @JsonAlias("CURRENTCONTROLLERID")
    private String currentControllerId;

    // 父端口类型
    @JsonProperty("homePortType")
    @JsonAlias("HOMEPORTTYPE")
    private String homePortType;

    @JsonProperty("ipV4Address")
    @JsonAlias("IPV4ADDR")
    private String ipV4Address;

    @JsonProperty("ipV6Address")
    @JsonAlias("IPV6ADDR")
    private String ipV6Address;
}
