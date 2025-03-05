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
package openbackup.tpops.protection.access.constant;

/**
 * 功能描述: GaussDb模块的通用常量
 *
 */
public class TpopsGaussDBConstant {
    /**
     * 允许接入的GaussDb集群规格上限
     */
    public static final int GAUSSDB_CLUSTER_MAX_COUNT = 8;

    /**
     * 常数项 0
     */
    public static final int INT_ZERO = 0;

    /**
     * 查询每页大小
     */
    public static final int QUERY_SIZE = 100;

    /**
     * GaussDB agents
     */
    public static final String GAUSSDB_AGENTS = "agents";

    /**
     * GaussDB agents
     */
    public static final String REGION = "region";

    /**
     * ens key 名称
     */
    public static final String REPOSITORIES_KEY_ENS = "esn";

    /**
     * repository role 角色 master
     */
    public static final int MASTER_ROLE = 0;

    /**
     * GaussDb集群的版本信息 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_VERSION = "clusterVersion";

    /**
     * GaussDb集群的版本信息 version
     */
    public static final String VERSION = "version";

    /**
     * GaussDb版本 version: tpops或convergent
     */
    public static final String DB_VERSION = "dbVersion";

    /**
     * GaussDb集群的使用状态 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_STATE = "status";

    /**
     * GaussDb集群的使用状态 扩展信息 extendInfo 中 实例状态 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_INSTANCE_STATUS = "instanceStatus";

    /**
     * GaussDb集群的使用状态 扩展信息 extendInfo 中 实例状态 的 Key 名称
     */
    public static final String EXTEND_INFO_VALUE_INSTANCE_ONLINE = "1";

    /**
     * GaussDb集群的使用状态 扩展信息 extendInfo 中 实例状态 的 Key 名称
     */
    public static final String EXTEND_INFO_VALUE_INSTANCE_OFFLINE = "0";

    /**
     * GaussDb集群的使用状态 扩展信息 extendInfo 中 实例状态为异常时 的 Key 名称
     */
    public static final String EXTEND_INFO_VALUE_INSTANCE_ABNORMAL = "2";

    /**
     * GaussDb 实例查询状态 表示实例正常
     */
    public static final String NORMAL_VALUE_STATE = "ACTIVE";

    /**
     * 在扩展信息中的extendInfo key 值 projectName
     */
    public static final String EXTEND_INFO_KEY_PROJECT_NAME = "projectName";

    /**
     * 在扩展信息中的extendInfo key 值 projectId
     */
    public static final String EXTEND_INFO_KEY_PROJECT_ID = "projectId";

    /**
     * 在扩展信息中的extendInfo key 值 projectId
     */
    public static final String EXTEND_INFO_KEY_PROJECT_ADDRESS = "pmAddress";

    /**
     * 在扩展信息中的extendInfo key 值 iamAccountName
     */
    public static final String EXTEND_INFO_KEY_ACCOUNT_NAME = "iamAccountName";

    /**
     * 在扩展信息中的extendInfo key 值 pmAddress
     */
    public static final String EXTEND_INFO_KEY_PM_ADDRESS = "pmAddress";

    /**
     * 多文件系统key名称
     */
    public static final String REPOSITORIES_KEY_MULTI_FILE_SYSTEM = "multiFileSystem";

    /**
     * python 传递的key BACKUP_METADATA_PATH
     */
    public static final String ADVANCE_PARAMS_KEY_BACKUP_METADATA_PATH = "backup_metadata_path";

    /**
     * 传递给Agent的key METADATA_PATH
     */
    public static final String ADVANCE_PARAMS_KEY_METADATA_PATH = "metadataPath";

    /**
     * python 传递的key storage_id
     */
    public static final String ADVANCE_PARAMS_KEY_STORAGE_ID = "storage_id";

    /**
     * 数据库 在扩展信息中的extendInfo key 值 GAUSSDB_USER
     */
    public static final String EXTEND_INFO_KEY_GAUSSDB_USER = "gaussdbUser";

    /**
     * 副本扩展信息中的extendInfo key 值 CAN_RESTORE
     */
    public static final String CAN_RESTORE = "canRestore";

    /**
     * python 传递的key BACKUP_TOOL_TYPE
     */
    public static final String ADVANCE_PARAMS_KEY_BACKUP_TOOL_TYPE = "backup_tool_type";

    /**
     * 传递给Agent的key TOOL_TYPE
     */
    public static final String ADVANCE_PARAMS_KEY_TOOL_TYPE = "backupToolType";

    /**
     * 速率统计 1为ubc统计，2为应用统计
     */
    public static final String SPEED_STATISTICS = "speedStatistics";

    /**
     * 待恢复的子类型
     */
    public static final String EXTEND_INFO_KEY_SUB_TYPE = "resourceSubType";

    /**
     * 恢复位置 key 名称
     */
    public static final String EXTEND_INFO_KEY_TARGET_LOCATION = "targetLocation";

    /**
     * PM下发给agent的扩展字段，用于判断日志备份是否需要转全量
     */
    public static final String IS_CHECK_BACKUP_JOB_TYPE = "isCheckBackupJobType";

    /**
     * 前端下发给PM的扩展字段，用于判断日志备份是否需要转全量
     */
    public static final String AUTO_FULL_BACKUP = "autoFullBackup";

    /**
     * 日志备份类型
     */
    public static final String LOG_BACKUP_TYPE = "logBackup";

    /**
     * 资源不支持恢复
     */
    public static final String DATABASE_RESTORE_FAIL_NOT_ALLOW_LABEL = "database_restore_not_allow_restore_label";

    /**
     * FALSE字符串
     */
    public static final String FALSE = "false";

    /**
     * 纳管GaussDb的TPOPS版本
     */
    public static final String TPOPS_VERSION = "tpopsVersion";

    /**
     * 注册项目信息时，密码错误
     */
    public static final String TPOPS_PASSWORD_ERROR = "tpops_password_error";
}
