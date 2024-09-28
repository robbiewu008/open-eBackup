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
package openbackup.data.protection.access.provider.sdk.archive;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;

/**
 * This interface defines the restore providers that need to be implemented by DataMover
 *
 */
public interface ArchiveImportProvider extends DataProtectionProvider<String> {
    /**
     * 云上保存归档副本，调用v1/dme_archive/updatesnap，失败重试，全部失败则记录jobInfo
     *
     * @param archiveObject archiveObject
     */
    void archiveUpdateSnap(ArchiveImportObject archiveObject);

    /**
     * 调用DME接口，扫描归档副本
     *
     * @param archiveObject archiveObject
     */
    void scanArchive(ArchiveImportObject archiveObject);

    /**
     * 归档副本扫描成功后，副本入库，dme清理临时数据
     *
     * @param taskCompleteMessage taskCompleteMessage
     * @param storageId storageId
     */
    void archiveImportTaskComplete(TaskCompleteMessageBo taskCompleteMessage, String storageId);

    /**
     * 归档副本扫描失败后，更新任务状态
     *
     * @param taskCompleteMessage taskCompleteMessage
     */
    void archiveImportTaskFailed(TaskCompleteMessageBo taskCompleteMessage);
}
