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

import lombok.Data;

import java.math.BigDecimal;

/**
 * 存储容量统计信息
 *
 */
@Data
public class StorageCapacitySummaryVo {
    /**
     * 类型
     */
    private String type;

    /**
     * 总容量(单位KB)
     */
    private BigDecimal totalCapacity = BigDecimal.ZERO;

    /**
     * 已使用容量(单位KB)
     */
    private BigDecimal usedCapacity = BigDecimal.ZERO;

    /**
     * 剩余容量(单位KB)
     */
    private BigDecimal freeCapacity = BigDecimal.ZERO;
}
