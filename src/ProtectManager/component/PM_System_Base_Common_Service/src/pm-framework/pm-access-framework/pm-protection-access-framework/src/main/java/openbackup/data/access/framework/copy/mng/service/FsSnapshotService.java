/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.service;

import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * 安全一体机创建文件系统快照接口
 *
 * @author q00564609
 * @since 2024-06-24
 * @version OceanCyber 300 1.2.0
 */
public interface FsSnapshotService {
    /**
     * 创建文件系统快照
     *
     * @param backupObject 备份信息
     * @param protectedResource 保护对象信息
     * @return 备份参数对象
     */
    BackupTask oceanCyberDetectBackup(BackupObject backupObject, ProtectedResource protectedResource);
}
