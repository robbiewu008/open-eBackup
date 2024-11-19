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
package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * 备份特性功能服务
 *
 */
public interface BackupFeatureService {
    /**
     * 是否支持数据备份与日志备份并行备份
     *
     * @param resourceId 资源ID
     * @return 是否支持并行
     */
    default boolean isSupportDataAndLogParallelBackup(String resourceId) {
        return isSupportDataAndLogParallelBackup(resourceId, false);
    }

    /**
     * 是否支持数据备份与日志备份并行备份
     *
     * @param resourceId 资源ID
     * @param isStrictMatch 是否严格匹配资源是否存在，为true时，若资源不存在，则抛出异常
     * @return 是否支持并行
     */
    boolean isSupportDataAndLogParallelBackup(String resourceId, boolean isStrictMatch);

    /**
     * 是否支持数据备份与日志备份并行备份
     *
     * @param resource 资源
     * @return 是否支持并行
     */
    boolean isSupportDataAndLogParallelBackup(ProtectedResource resource);

    /**
     * 转换备份类型
     *
     * @param backupType 备份类型
     * @param resourceId 资源ID
     * @return 转换后的备份类型
     */
    BackupTypeConstants transferBackupType(BackupTypeConstants backupType, String resourceId);

    /**
     * 转换备份类型
     *
     * @param backupType 备份类型
     * @param resource 资源
     * @return 转换后的备份类型
     */
    BackupTypeConstants transferBackupType(BackupTypeConstants backupType, ProtectedResource resource);
}
