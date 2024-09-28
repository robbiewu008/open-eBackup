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
package openbackup.system.base.common.model.job;

/**
 * Job 类型枚举类
 * （计划删除，请用com.huawei.oceanprotect.system.base.sdk.job.model.JobTypeEnum）
 *
 * @see openbackup.system.base.sdk.job.model.JobTypeEnum
 */
@Deprecated
public enum JobTypeEnum {
    /**
     * SYSTEM
     */
    SYSTEM,

    /**
     * BACKUP
     */
    BACKUP,

    /**
     * RECOVERY
     */
    RESTORE,

    /**
     * LIVE_RECOVERY
     */
    INSTANT_RESTORE,

    /**
     * CLOUD_ARCHIVE_RESTORE
     */
    CLOUD_ARCHIVE_RESTORE,

    /**
     * LIVE_MOUNT
     */
    LIVE_MOUNT,

    /**
     * DUPLICATE
     */
    COPY_REPLICATION,

    /**
     * ARCHIVE
     */
    ARCHIVE,

    /**
     * DELETE_REPLICA
     */
    DELETE_COPY
}
