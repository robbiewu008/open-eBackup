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
package openbackup.gaussdbdws.protection.access.service;

import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryProvider;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.fasterxml.jackson.databind.JsonNode;
import com.google.common.collect.ImmutableList;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * DWS
 *
 */
@Component
@AllArgsConstructor
@Slf4j
public class DwsStorageRepositoryProvider implements StorageRepositoryProvider {
    private static final ImmutableList<ResourceSubTypeEnum> DWS_SUBTYPES =
        ImmutableList.of(ResourceSubTypeEnum.GAUSSDB_DWS, ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE,
            ResourceSubTypeEnum.GAUSSDB_DWS_TABLE, ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA);

    private final BackupStorageApi backupStorageApi;

    private final StorageRepositoryCreateService storageRepositoryCreateService;

    @Override
    public List<StorageRepository> buildBackupDataRepository(BackupObject backupObject) {
        log.info("Dws parallel is enabled.Job_id: {}", backupObject.getRequestId());
        return storageRepositoryCreateService.createRepositoryByStorageUnitGroup(backupObject.getPolicy()
            .getExtParameters()
            .get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)
            .get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY)
            .textValue());
    }

    @Override
    public boolean applicable(BackupObject object) {
        // DWS开启并行存储后，需要单独处理
        if (DWS_SUBTYPES.stream()
            .noneMatch(subType -> subType.equalsSubType(object.getProtectedObject().getSubType()))) {
            return false;
        }
        JsonNode extParameters = object.getPolicy().getExtParameters();
        if (!isBoundStorageUnitGroup(extParameters)) {
            return false;
        }
        return backupStorageApi.getDetail(extParameters.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)
            .get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY)
            .textValue()).isHasEnableParallelStorage();
    }

    private boolean isBoundStorageUnitGroup(JsonNode extParameters) {
        if (!extParameters.has(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)) {
            return false;
        }
        JsonNode storageInfo = extParameters.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY);
        return storageInfo.has(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY)
            && BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE
                .equals(storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY).textValue());
    }
}
