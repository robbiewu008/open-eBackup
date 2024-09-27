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
 * 整系统存储信息
 *
 * @author h30003246
 * @since 2020-06-23
 */
@Data
public class ClusterStorageVo {
    private Integer clusterId;

    private String esn;

    private BigDecimal totalCapacity;

    private BigDecimal usedCapacity;

    private BigDecimal freeCapacity;

    private BigDecimal writeCapacity;

    private BigDecimal consumedCapacity;

    private BigDecimal spaceReductionRate;

    private String logic;

    /**
     * ClusterCapacityInfo to ClusterStorageVo
     *
     * @param info ClusterCapacityInfo
     * @return ClusterStorageVo
     */
    public static ClusterStorageVo of(ClusterCapacityInfo info) {
        ClusterStorageVo vo = new ClusterStorageVo();
        vo.setClusterId(info.getClusterId());
        vo.setEsn(info.getEsn());
        vo.setTotalCapacity(info.getTotalCapacity());
        vo.setUsedCapacity(info.getUsedCapacity());
        vo.setFreeCapacity(info.getFreeCapacity());
        vo.setWriteCapacity(info.getWriteCapacity());
        vo.setConsumedCapacity(info.getConsumedCapacity());
        vo.setSpaceReductionRate(info.getSpaceReductionRate());
        vo.setLogic(info.getLogic());
        return vo;
    }
}
