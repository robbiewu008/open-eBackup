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

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupFeatureService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.ResourceNotExistException;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Optional;

/**
 * 备份特性功能服务 实现类
 *
 */
@Slf4j
@Service
public class BackupFeatureServiceImpl implements BackupFeatureService {
    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ProviderManager providerManager;

    @Override
    public boolean isSupportDataAndLogParallelBackup(String resourceId, boolean isStrictMatch) {
        Optional<ProtectedResource> resource = resourceService.getBasicResourceById(resourceId);
        if (!resource.isPresent()) {
            log.warn("Protected resource is not found. resourceId is {}", resourceId);
            if (isStrictMatch) {
                throw new ResourceNotExistException(CommonErrorCode.OBJ_NOT_EXIST, "resource is not found");
            }
            return false;
        }
        return isSupportDataAndLogParallelBackup(resource.get());
    }

    @Override
    public BackupTypeConstants transferBackupType(BackupTypeConstants backupType, String resourceId) {
        ProtectedResource resource = resourceService.getBasicResourceById(resourceId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Resource not found."));
        return transferBackupType(backupType, resource);
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        BackupInterceptorProvider backupInterceptorProvider =
                providerManager.findProvider(BackupInterceptorProvider.class, resource.getSubType(), null);
        if (backupInterceptorProvider == null) {
            return false;
        }
        return backupInterceptorProvider.isSupportDataAndLogParallelBackup(resource);
    }

    @Override
    public BackupTypeConstants transferBackupType(BackupTypeConstants backupType, ProtectedResource resource) {
        BackupInterceptorProvider backupInterceptorProvider =
                providerManager.findProvider(BackupInterceptorProvider.class, resource.getSubType(), null);
        if (backupInterceptorProvider == null) {
            return backupType;
        }
        return backupInterceptorProvider.transferBackupType(backupType, resource);
    }
}
