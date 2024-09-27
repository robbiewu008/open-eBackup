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

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.math.BigDecimal;

/**
 * 存储单元VO
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-06
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class StorageUnitVo {
    private String id;

    /**
     * 关联TargetCluster.remoteEsn
     */
    private String deviceId;

    private String poolId;

    private String name;

    private String deviceType;

    private String poolName;

    private BigDecimal totalCapacity;

    private BigDecimal usedCapacity;

    private String threshold;

    private Integer healthStatus;

    private Integer runningStatus;

    private String deviceName;

    @JsonProperty("isAutoAdded")
    private Boolean isAutoAdded;

    private Integer generatedType;

    /**
     * 关联TargetCluster.clusterId
     */
    private Integer clusterId;

    private double spaceReductionRate;
}
