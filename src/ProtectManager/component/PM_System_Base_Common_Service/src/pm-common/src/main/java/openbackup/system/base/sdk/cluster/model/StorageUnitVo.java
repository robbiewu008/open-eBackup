/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2024. All rights reserved.
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
