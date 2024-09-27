/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.archive;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;

/**
 * This interface defines the restore providers that need to be implemented by DataMover
 *
 * @author z30009433
 * @since 2020-12-30
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
