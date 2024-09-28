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

import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.util.Routing;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AccessLevel;
import lombok.Data;
import lombok.Getter;
import lombok.Setter;

import java.net.URI;
import java.util.List;
import java.util.function.Consumer;
import java.util.function.Function;

/**
 * target cluster object
 *
 */
@Data
public class TargetClusterVo {
    private String clusterId;

    private List<String> mgrIpList;

    private int port;

    private String username;

    private String password;

    private int status;

    private DataMoverIps ips;

    private String esn;

    @JsonProperty("netplaneinfo")
    private List<NodePodInfo> netplaneInfo;

    @Getter(AccessLevel.NONE)
    @Setter(AccessLevel.NONE)
    @JsonIgnore
    private Routing routing;

    private String deployType;

    private ClusterDetailInfo clusterDetailInfo;

    /**
     * get method
     *
     * @param function function
     * @param <T> type
     * @return result
     */
    public <T> T get(Function<URI, T> function) {
        return routing().get(function);
    }

    /**
     * run method
     *
     * @param function function
     * @param <T> type
     * @return result
     */
    public <T> T call(Function<URI, T> function) {
        return routing().call(function);
    }

    /**
     * run method
     *
     * @param consumer consumer
     */
    public void run(Consumer<URI> consumer) {
        call(
                uri -> {
                    consumer.accept(uri);
                    return null;
                });
    }

    private synchronized Routing routing() {
        if (routing == null) {
            routing = new Routing(mgrIpList, port);
        }
        return routing;
    }
}
