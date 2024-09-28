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
package openbackup.data.access.client.sdk.api.framework.dme;

import lombok.Data;

/**
 * 指定时间范围可用于恢复的时间段
 *
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
