/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.model.job;

/**
 * Job 类型枚举类
 * （计划删除，请用com.huawei.oceanprotect.system.base.sdk.job.model.JobTypeEnum）
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-04-15
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
