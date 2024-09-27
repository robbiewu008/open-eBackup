/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.backup.service;

import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;

/**
 * 备份服务，处理备份请求
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-25
 */
public interface IBackupService {
    /**
     * 执行备份
     *
     * @param backupObject 备份对象{@link BackupObject}
     * @return 备份任务对象
     */
    BackupTask backup(BackupObject backupObject);
}
