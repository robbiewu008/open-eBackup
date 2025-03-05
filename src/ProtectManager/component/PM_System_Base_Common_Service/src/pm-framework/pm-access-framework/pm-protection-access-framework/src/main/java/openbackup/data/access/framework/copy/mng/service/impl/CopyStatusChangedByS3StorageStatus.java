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

import com.huawei.oceanprotect.repository.s3.entity.S3StorageStatus;
import com.huawei.oceanprotect.repository.service.impl.S3StorageStatusChangedObserver;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.system.base.common.model.repository.enums.StoragePoolRunningStatus;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyStorageUnitStatus;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Copy S3 Storage Status observer
 *
 */
@Slf4j
@Component
public class CopyStatusChangedByS3StorageStatus implements S3StorageStatusChangedObserver {
    private static final ImmutableList<Integer> ALLOW_COPY_ONLINE_STORAGE_STATUS = ImmutableList.of(
            StoragePoolRunningStatus.ONLINE.getOnlineStatus(),
            StoragePoolRunningStatus.PARTIALLY_ONLINE.getOnlineStatus());

    @Autowired
    CopyManagerService copyManagerService;

    @Autowired
    CopyRestApi copyRestApi;

    /**
     * 存储状态变化，修改副本存储单元状态
     *
     * @param s3StorageStatus 存储单元状态
     */
    @Override
    public void onS3StorageStatusChanged(S3StorageStatus s3StorageStatus) {
        log.info("storage status changed, and copies status will be changed");
        List<String> copyIdList = copyManagerService
                .queryCopyIdByStorageId(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(), s3StorageStatus.getStorageId());
        log.info("update copies status in storage pool: {}", s3StorageStatus.getStorageId());
        if (VerifyUtil.isEmpty(copyIdList)) {
            return;
        }
        if (ALLOW_COPY_ONLINE_STORAGE_STATUS.contains(s3StorageStatus.getStatus())) {
            copyManagerService.updateCopyStorageUnitStatus(copyIdList, CopyStorageUnitStatus.ONLINE.getValue());
        } else {
            copyManagerService.updateCopyStorageUnitStatus(copyIdList, CopyStorageUnitStatus.OFFLINE.getValue());
        }
    }
}