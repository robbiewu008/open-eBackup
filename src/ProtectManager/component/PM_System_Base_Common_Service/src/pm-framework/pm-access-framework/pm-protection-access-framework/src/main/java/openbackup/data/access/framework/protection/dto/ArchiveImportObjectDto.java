/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author z30009433
 * @since 2020-12-30
 */
@Builder
@AllArgsConstructor
@NoArgsConstructor
@Data
public class ArchiveImportObjectDto {
    private String storageId;

    private String requestId;

    private String archiveCopyId;

    private String backupCopyId;

    private String objectType;

    private String policy;

    private String jobId;

    private int autoRetryTimes;
}
