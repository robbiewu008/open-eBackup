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

    // 物理用量
    private BigDecimal usedCapacity;

    // 逻辑用量
    private BigDecimal usedLogicCapacity;

    private String threshold;

    /**
     * 存储池容量严重不足告警阈值
     */
    private int majorThreshold;

    /**
     * 存储池容量即将耗尽告警阈值
     */
    private int endingUpThreshold;

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
