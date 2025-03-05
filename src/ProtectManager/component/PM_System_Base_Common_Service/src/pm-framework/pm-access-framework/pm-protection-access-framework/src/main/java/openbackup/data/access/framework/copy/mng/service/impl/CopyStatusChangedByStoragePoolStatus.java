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
package openbackup.data.access.framework.copy.mng.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.entity.StoragePoolInfo;
import com.huawei.oceanprotect.base.cluster.sdk.service.StoragePoolStatusChangedObserver;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.system.base.common.model.repository.enums.StoragePoolHealthStatus;
import openbackup.system.base.common.model.repository.enums.StoragePoolRunningStatus;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyStorageUnitStatus;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Copy Storage pool Status observer
 *
 */
@Slf4j
@Component
public class CopyStatusChangedByStoragePoolStatus implements StoragePoolStatusChangedObserver {
    private static final ImmutableList<String> ALLOW_COPY_ONLINE_RUNNING_STATUS = ImmutableList.of(
            StoragePoolRunningStatus.ONLINE.getRunningStatus(),
            StoragePoolRunningStatus.RECONSTRUCTION.getRunningStatus(),
            StoragePoolRunningStatus.PRE_COPY.getRunningStatus(),
            StoragePoolRunningStatus.BALANCING.getRunningStatus(),
            StoragePoolRunningStatus.DEGRADED.getRunningStatus());

    private static final ImmutableList<String> ALLOW_COPY_ONLINE_HEALTH_STATUS = ImmutableList
            .of(StoragePoolHealthStatus.NORMAL.getHealthStatus(), StoragePoolHealthStatus.DEGRADED.getHealthStatus());

    @Autowired
    CopyManagerService copyManagerService;

    @Autowired
    CopyMapper copyMapper;

    @Autowired
    StorageUnitService storageUnitService;

    /**
     * 存储池状态变化，修改副本状态
     *
     * @param storagePoolInfoList 存储池列表
     */
    @Override
    public void onStoragePoolStatusChanged(List<StoragePoolInfo> storagePoolInfoList) {
        log.info("storage pool status changed, and copies status will be changed");
        for (StoragePoolInfo storagePoolInfo : storagePoolInfoList) {
            String storageUnitId = storageUnitService.getStorageUnitIdByStoragePool(storagePoolInfo.getDeviceId(),
                    storagePoolInfo.getPoolId());
            List<String> copyIdList = copyMapper.selectCopyIdByStorageUnitId(
                    CopyGeneratedByEnum.COPY_GENERATED_BY_REPLICATION_AND_BACKUP_AND_LIVE_MOUNT, storageUnitId);
            if (VerifyUtil.isEmpty(copyIdList)) {
                return;
            }
            log.info("update copies status in storage unit: {}", storageUnitId);
            if (ALLOW_COPY_ONLINE_RUNNING_STATUS.contains(storagePoolInfo.getRunningStatus())
                    && ALLOW_COPY_ONLINE_HEALTH_STATUS.contains(storagePoolInfo.getHealthStatus())) {
                copyManagerService.updateCopyStorageUnitStatus(copyIdList, CopyStorageUnitStatus.ONLINE.getValue());
            } else {
                copyManagerService.updateCopyStorageUnitStatus(copyIdList, CopyStorageUnitStatus.OFFLINE.getValue());
            }
        }
    }
}