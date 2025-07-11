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
package openbackup.data.protection.access.provider.sdk.backup;

/**
 * 资源扩展字段的常量
 *
 */
public class ResourceExtendInfoConstants {
    /**
     * 下次备份类型。在保护对象ext的key
     *
     * 值为 BackupTypeEnum的小写。 例如 full
     */
    public static final String NEXT_BACKUP_TYPE_EXT_KEY = "next_backup_type";

    /**
     * 下次备份类型引发原因。在保护对象ext的key
     *
     * 值为 NextBackupChangeCauseEnum。 例如 LOG_BACKUP_SUCCESS
     */
    public static final String NEXT_BACKUP_CHANGE_CAUSE_EXT_KEY = "next_backup_change_cause";

    /**
     * 多集群Agent连通性。
     *
     * 值为 Map<String, AgentConnectionResult> ,key: esn
     */
    public static final String CONNECTION_RESULT_KEY = "connection_result";
}
