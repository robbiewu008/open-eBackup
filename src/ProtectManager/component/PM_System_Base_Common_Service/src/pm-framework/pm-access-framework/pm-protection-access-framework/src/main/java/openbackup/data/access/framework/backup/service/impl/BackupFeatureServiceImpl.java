/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.framework.backup.service.impl;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupFeatureService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Optional;

/**
 * 备份特性功能服务 实现类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/5/27
 */
@Slf4j
@Service
public class BackupFeatureServiceImpl implements BackupFeatureService {
    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ProviderManager providerManager;

    @Override
    public boolean isSupportDataAndLogParallelBackup(String resourceId) {
        Optional<ProtectedResource> resource = resourceService.getBasicResourceById(resourceId);
        if (!resource.isPresent()) {
            log.warn("Protected resource is not found. resourceId is {}", resourceId);
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
