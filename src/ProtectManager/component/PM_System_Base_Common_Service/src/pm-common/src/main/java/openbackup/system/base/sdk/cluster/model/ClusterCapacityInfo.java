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
