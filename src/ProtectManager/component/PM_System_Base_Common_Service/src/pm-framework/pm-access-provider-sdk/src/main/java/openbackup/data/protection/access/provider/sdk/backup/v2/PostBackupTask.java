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
