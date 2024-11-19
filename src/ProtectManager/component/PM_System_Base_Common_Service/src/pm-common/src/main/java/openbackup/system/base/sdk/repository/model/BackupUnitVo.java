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
package openbackup.system.base.sdk.repository.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;

import org.springframework.beans.BeanUtils;

import java.math.BigDecimal;

/**
 * 存储单元信息
 *
 */
@Setter
@Getter
@AllArgsConstructor
@NoArgsConstructor
public class BackupUnitVo {
    private String unitId;

    private String unitName;

    // 备份存储单元在指定策略下的顺序
    private Integer strategyOrder;

    // 可使用容量阀值 0~100%
    private Integer availableCapacityRatio;

    private String deviceId;

    private String poolId;

    private String deviceType;

    private String poolName;

    private BigDecimal totalCapacity;

    private BigDecimal usedCapacity;

    private String threshold;

    private Integer healthStatus;

    private Integer runningStatus;

    private String deviceName;

    private boolean isAutoAdded;

    private BackupClusterVo backupClusterVo;

    /**
     * 构造函数
     *
     * @param storageUnitVo StorageUnitVo
     */
    public BackupUnitVo(StorageUnitVo storageUnitVo) {
        BeanUtils.copyProperties(storageUnitVo, this);
        this.unitId = storageUnitVo.getId();
        this.unitName = storageUnitVo.getName();
    }

    /**
     * 映射成备份存储单元视图对象
     *
     * @return 备份存储单元视图
     */
    public StorageUnitVo toStorageUnitVo() {
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        BeanUtils.copyProperties(this, storageUnitVo);
        storageUnitVo.setId(this.unitId);
        storageUnitVo.setName(this.unitName);
        return storageUnitVo;
    }
}
