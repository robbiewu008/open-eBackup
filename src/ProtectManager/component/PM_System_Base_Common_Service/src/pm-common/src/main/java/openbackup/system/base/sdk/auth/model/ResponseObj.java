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
package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 调用hcs接口，根据条件查询浮动ip信息响应体中的对象，实际只使用了externalRelayIpAddress
 *
 */
@Getter
@Setter
public class ResponseObj {
    private String ownerType;
    private String tenantType;
    private String extraSpecs;
    private String privateIps;
    private String ownerId;
    private String deviceName;
    private String createdAt;
    private String diskSize;
    private String taskState;
    private String powerState;
    private String confirmStatus;
    private String id;
    private String podNativeId;
    private String podId;
    private String bizRegionId;
    private String floatingIp;
    private String imageId;
    @JsonProperty("class_Id")
    private Long classId;
    private String originalState;
    private String ipAddress;
    private String hostId;
    private String hypervisorType;
    private String resId;
    private String tags;
    @JsonProperty("is_Local")
    private Boolean isLocal;
    @JsonProperty("class_Name")
    private String className;
    private String azoneId;
    private String physicalHostId;
    private String resourcePoolId;
    private String flavorPerformanceType;
    private String name;
    private Boolean isVirtual;
    private String projectId;
    private String bizRegionName;
    private String resourcePoolType;
    private String status;
    private String logicalRegionName;
    private String azoneName;
    private String managedStatus;
    private String regionName;
    private String flavorId;
    private String clusterId;
    private String dataPlane;
    private String ownerName;
    private String tenantName;
    private String clusterName;
    private String osType;
    @JsonProperty("last_Modified")
    private Long lastModified;
    private String vdcId;
    private String resourcePoolName;
    private String userId;
    private String keystoneId;
    private String logicalRegionId;
    private String regionId;
    private String createTime;
    @JsonProperty("management")
    private Boolean isManagement;
    private String tenantId;
    private String vdcName;
    private String nativeId;
    private String launchedAt;
    private String vmState;
    private String cpuCoreNum;

    // 外部中继Ip地址
    private String externalRelayIpAddress;
}
