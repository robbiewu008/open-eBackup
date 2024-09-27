/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme;

import lombok.Data;

/**
 * 指定时间范围可用于恢复的时间段
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-26
 */
@Data
public class AvailableTimeRanges {
    /**
     * 副本id
     */
    private String copyId;

    /**
     * 起始时间点
     */
    private long startTime;

    /**
     * 结束时间
     */
    private long endTime;

    /**
     * 可恢复时间范围
     * 当数据库插件需上报多个可恢复时间区段时，将时间戳以二维数组的形式传至该字段
     * 例：[[1685155711, 1685173850],[1685173956, 1685175203]]
     */
    private String timeRange;
}