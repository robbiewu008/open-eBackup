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
package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * om-存储控制器节点信息
 *
 */
@Data
public class NodeDetail {
    //  k8s节点管理ip / 节点外部ip
    @JsonProperty(value = "management_address")
    private String managementAddress;

    // k8s节点内部ip
    private String address;

    // k8s高可用ip
    @JsonProperty(value = "control_plane_endpoint")
    private String controllerPlaneEndpoint;

    // gaussDB高可用ip
    @JsonProperty(value = "ha_endpoint")
    private String haEndpoint;

    // k8s节点集群浮动ip
    @JsonProperty(value = "service_plane_endpoint")
    private String servicePlaneEndpoint;

    private List<String> componentList;

    @JsonProperty(value = "hostname")
    private String hostName;

    private String nodeName;

    private String nodeStatus;

    private String role;
}
