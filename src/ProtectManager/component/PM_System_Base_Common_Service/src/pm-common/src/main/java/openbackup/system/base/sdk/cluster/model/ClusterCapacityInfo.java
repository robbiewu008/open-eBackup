/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import java.math.BigDecimal;

/**
 * 目标集群容量信息查询数据返回模型
 *
 * @author dWX1009286
 * @since 2021-07-20
 */
@Data
public class ClusterCapacityInfo {
    private Integer clusterId;

    private String esn;

    private BigDecimal totalCapacity = BigDecimal.ZERO;

    private BigDecimal usedCapacity = BigDecimal.ZERO;

    private BigDecimal freeCapacity = BigDecimal.ZERO;

    private BigDecimal writeCapacity = BigDecimal.ZERO;

    private BigDecimal consumedCapacity = BigDecimal.ZERO;

    private BigDecimal spaceReductionRate = BigDecimal.ZERO;

    private String logic;
}
