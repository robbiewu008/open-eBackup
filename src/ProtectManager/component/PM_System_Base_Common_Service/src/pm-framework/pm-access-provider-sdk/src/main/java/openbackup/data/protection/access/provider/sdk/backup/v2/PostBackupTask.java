/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.protection.model.PolicyBo;

import lombok.Getter;
import lombok.Setter;

/**
 * 通用备份框架后置备份任务
 *
 * @since 2022-10-13
 */
@Setter
@Getter
public class PostBackupTask {
    ProtectedObject protectedObject;
    ProviderJobStatusEnum jobStatus;
    BackupTypeEnum backupType;
    CopyInfo copyInfo;
    PolicyBo policyBo;
    String userId;
}
