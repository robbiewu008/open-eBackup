/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.data.protection.access.provider.sdk.backup;

/**
 * 资源扩展字段的常量
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-13
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
