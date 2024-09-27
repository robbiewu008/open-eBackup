/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.data.access.framework.backup.constant;

/**
 * 备份相关常量
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-11-09
 */
public class BackupConstant {
    /**
     * 下次备份变更的原因的默认值0，表示没有变更
     */
    public static final String BACKUP_EXT_PARAM_NEXT_CAUSE_DEFAULT_VALUE = "0";

    /**
     * 下次备份变更的原因key, value为NextBackupChangeCauseEnum.index,
     *  如果没有原因，则为0
     */
    public static final String BACKUP_EXT_PARAM_NEXT_CAUSE_KEY = "next_cause_param";

    /**
     * 全量 label
     */
    public static final String FULL_LABEL = "protection_full_label";

    /**
     * 增量 label
     */
    public static final String INCREMENTAL_LABEL = "protection_incremental_label";

    /**
     * 差异 label
     */
    public static final String DIFF_LABEL = "common_diff_label";

    /**
     * 日志 label
     */
    public static final String LOG_LABEL = "common_log_label";

    /**
     * 备份的SLA策略里的备份存储信息KEY
     */
    public static final String BACKUP_EXT_PARAM_STORAGE_INFO_KEY = "storage_info";

    /**
     * 备份的SLA策略里的备份存储类型KEY
     */
    public static final String BACKUP_EXT_PARAM_STORAGE_TYPE_KEY = "storage_type";

    /**
     * 备份的SLA策略里的备份存储ID KEY
     */
    public static final String BACKUP_EXT_PARAM_STORAGE_ID_KEY = "storage_id";

    /**
     * 备份的SLA策略里的备份存储单元组value
     */
    public static final String BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE = "storage_unit_group";

    /**
     * 备份的SLA策略里的备份存储单元value
     */
    public static final String BACKUP_EXT_PARAM_STORAGE_UNIT_VALUE = "storage_unit";

    /**
     * 备份SLA里并行备份开关
     */
    public static final String MULTI_NODE_BACKUP_SWITCH = "multiNodeBackupSwitch";

    /**
     * agent连通的网络平面/逻辑端口 IP
     */
    public static final String AGENT_CONNECTED_IPS = "agentConnectedIps";

    /**
     * 断点续传的默认重试次数
     */
    public static final int DEFAULT_CHECK_POINT_RETRY_NUM = 3;

    /**
     * 断点续传的最小重试次数
     */
    public static final int MIN_CHECK_POINT_RETRY_NUM = 1;

    /**
     * 断点续传的最大重试次数
     */
    public static final int MAX_CHECK_POINT_RETRY_NUM = 5;
}
