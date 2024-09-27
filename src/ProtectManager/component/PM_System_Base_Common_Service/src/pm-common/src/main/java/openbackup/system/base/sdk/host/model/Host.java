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
package openbackup.system.base.sdk.host.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * host对象
 *
 * @author t00482481
 * @since 2020-09-20
 */
@Data
public class Host {
    /**
     * host offline
     */
    public static final String HOST_OFFLINE = "0";

    @JsonProperty("host_id")
    private String hostId;

    private String name;

    @JsonProperty("env_type")
    private String envType;

    @JsonProperty("endpoint")
    private String ip;

    private String port;

    @JsonProperty("link_status")
    private String linkStatus;

    @JsonProperty("os_type")
    private String osType;

    @JsonProperty("proxy_type")
    private String proxyType;

    @JsonProperty("is_cluster")
    private boolean isCluster;

    @JsonProperty("cluster_info")
    private List<String> clusterInfo;
}
