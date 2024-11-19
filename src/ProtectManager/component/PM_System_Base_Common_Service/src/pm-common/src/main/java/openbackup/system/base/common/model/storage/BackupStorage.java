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
package openbackup.system.base.common.model.storage;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 存储信息
 *
 */
@Data
public class BackupStorage {
    /**
     * dme规定类型 vmware和oracle默认为1
     */
    @JsonProperty("Type")
    private int type = 1;

    @JsonProperty("IP")
    private String ip;

    @JsonProperty("IPV6IP")
    private String ipv6ip;

    @JsonProperty("Port")
    private int port;

    @JsonProperty("Username")
    private String username;

    @JsonProperty("Password")
    private String password;

    /**
     * dme规定 vmware和oracle默认为0
     */
    @JsonProperty("PoolId")
    private int poolId = 0;
}
