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
