/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 储容量百分比和颜色显示
 *
 * @author z00613137
 * @since 2023-05-11
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class ClusterStorageTendencyDayInfo {
    private double totalCapacity;

    private double usedCapacity;

    private double percentage;

    private long timestamp;
}