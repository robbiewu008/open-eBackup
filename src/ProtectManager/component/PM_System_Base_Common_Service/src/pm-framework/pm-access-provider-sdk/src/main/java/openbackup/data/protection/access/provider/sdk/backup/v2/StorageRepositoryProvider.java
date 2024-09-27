/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;

import java.util.List;

/**
 * 功能描述
 *
 * @author w30044259
 * @since 2024-03-26
 */
public interface StorageRepositoryProvider extends DataProtectionProvider<BackupObject> {
    /**
     * 构造数据仓
     *
     * @param backupObject backupObject
     * @return  repository
     */
    List<StorageRepository> buildBackupDataRepository(BackupObject backupObject);
}
