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
package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.Locale;

/**
 * 资源扩展字段参数
 *
 */
@Getter
@Setter
public class NextBackupParams {
    /**
     * 下次备份类型。在保护对象ext的key
     *
     * 值为 BackupTypeEnum的小写。 例如 full
     */
    @JsonProperty("next_backup_type")
    private String nextBackupType;

    /**
     * 下次备份类型引发原因。在保护对象ext的key
     *
     * 值为 NextBackupChangeCauseEnum。 例如 LOG_BACKUP_SUCCESS
     */
    @JsonProperty("next_backup_change_cause")
    private String nextBackupChangeCause;

    public NextBackupParams(String nextBackupChangeCause) {
        this(BackupTypeEnum.FULL.name().toLowerCase(Locale.ROOT), nextBackupChangeCause);
    }

    public NextBackupParams(String nextBackupType, String nextBackupChangeCause) {
        this.nextBackupType = nextBackupType;
        this.nextBackupChangeCause = nextBackupChangeCause;
    }
}
