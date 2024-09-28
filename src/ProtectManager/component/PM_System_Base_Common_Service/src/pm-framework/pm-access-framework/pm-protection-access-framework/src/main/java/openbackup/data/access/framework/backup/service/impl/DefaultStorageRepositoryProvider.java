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
package openbackup.data.access.framework.backup.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.dto.StorageUnitQueryParam;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryProvider;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.service.DeployTypeService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * 框架提供的默认构造存储仓的方法
 *
 */
@Component
@AllArgsConstructor
@Slf4j
public class DefaultStorageRepositoryProvider implements StorageRepositoryProvider {
    private final JobService jobService;

    private StorageRepositoryCreateService storageRepositoryCreateService;

    private StorageUnitService storageUnitService;

    private DeployTypeService deployTypeService;

    @Override
    public List<StorageRepository> buildBackupDataRepository(BackupObject backupObject) {
        if (!isNeedBuildRepository()) {
            log.info("Current deploy type doesn't need build repository. Job id: {}", backupObject.getRequestId());
            return Collections.emptyList();
        }
        // 任务分发会将选好的存储单元保存到任务信息里
        String storageUnitId = jobService.queryJob(backupObject.getRequestId()).getStorageUnitId();
        // 分布式、cloud backup、安全一体机等不需要分发的部署形态，构造默认的存储库，只有一个存储单元
        if (VerifyUtil.isEmpty(storageUnitId)) {
            List<StorageUnitVo> unitVos =
                storageUnitService.pageQueryStorageUnits(new StorageUnitQueryParam(), 0, 1).getRecords();
            if (VerifyUtil.isEmpty(unitVos)) {
                log.warn("No storage unit found for job. Job id: {}", backupObject.getRequestId());
                return Collections.emptyList();
            }
            storageUnitId = unitVos.get(0).getId();
            log.info("Job dispatch is invalid. Use default storage unit: {}", storageUnitId);
        }
        return Collections.singletonList(storageRepositoryCreateService.createRepositoryByStorageUnit(storageUnitId));
    }

    private boolean isNeedBuildRepository() {
        return !deployTypeService.isCyberEngine();
    }

    @Override
    public boolean applicable(BackupObject backupObject) {
        return false;
    }
}
