#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

from enum import Enum
from common.env_common import get_install_head_path
from common.const import RpcParamKey


class BackupGranEnum(str, Enum):
    # br工具中的备份类型
    CLUSTER_BACKUP = 'TiDB-cluster'
    DB_BACKUP = 'TiDB-database'
    TABLE_BACKUP = 'TiDB-table'


class EnvName:
    IAM_USERNAME = ""
    IAM_PASSWORD = ""
    IAM_BELONG = ""


class TiDBSubType:
    # TiDB涉及的类型
    TYPE = "Database"
    # br工具中的备份类型
    SUBTYPE_CLUSTER = "TiDB-cluster"
    SUBTYPE_DATABASE = "TiDB-database"
    SUBTYPE_TABLE = "TiDB-table"


class TiDBRegisterActionType:
    # TiDB注册时的不同动作
    CHECK_TIUP_LIST_CLUSTER = "check_tiup"
    LIST_HOSTS = "get_cluster_info"
    CHECK_USER = "check_cluster"
    LIST_DB = "list_db"
    LIST_TABLE = "list_table"
    # TiDB健康性检查时的不同动作
    CHECK_CLUSTER = "check_cluster"
    CHECK_DB = "check_db"
    CHECK_TABLE = "check_table"


class ErrorCode(int, Enum):
    # 错误码编号待定
    # 集群节点数不一致
    ERROR_DIFFERENT_TOPO = 1577209972
    # 数据库用户名不一致
    ERROR_DIFFERENT_USER = 1577209973
    # 数据库状态异常
    ERR_DATABASE_STATUS = 1577210000
    # 某个服务未正常开启，返回参数 服务名称
    ERROR_SERVICE = 1577213479
    # 节点的ip和agent不一致
    ERROR_PARAM = 1677929218
    # 认证信息错误
    ERROR_AUTH = 1577209942
    # 输入参数错误
    ERR_INPUT_STRING = 1677934101
    # 检查数据库用户权限不足
    CHECK_PRIVILEGE_FAILED = 1677873187
    # 集群不存在
    CLUSTER_NOT_EXIST = 1677929991
    # TiUP校验失败
    CHECK_TIUP_FAILED = 1677873215
    # TiDB用户备份权限不足
    TIDB_USER_PRIVILEGE_FAILED = 1677873222
    # TiDB集群日志备份未开启
    CHECK_LOG_FAILED = 1677873216
    # TiDB表不存在
    TIDB_TABLE_NOT_EXIST = 1677873221
    # TiDB数据库不存在
    TIDB_DB_NOT_EXIST = 1677873220
    # TiDB集群不存在
    TIDB_CLUSTER_NOT_EXIST = 1677873217
    # TiKV或者TiFlash主机与集群不一致
    TIKV_TIFLASH_DIFFERENT = 1677873219
    # PD或TiDB主机不在线
    CHECK_PD_TIDB_FAILED = 1677873218
    # 备份路径异常，不存在，权限，属主不正确
    ERR_DIRECTORY = 1677873240
    # 复制备份文件失败
    ERR_COPY = 1677873238
    # 日志备份时间异常
    ERR_TIME_INVALID = 1677873237
    # 日志备份状态异常
    ERR_LOG_BKP_STATUS = 1677873236
    # 待备份的数据库或表不存在
    BKP_DB_TAB_NOT_EXIST = 1677873235
    # 执行备份/恢复命令失败
    EXEC_BACKUP_RECOVER_CMD_FAIL = 1577209989
    # 获取BR版本失败
    ERR_GET_BR_VERSION = 1677873241
    # 主机不在线
    HOST_NOT_UP = 1677873242
    # uid不一致
    UID_INCONSISTENCY = 1677873180
    # 数据库环境异常不支持备份恢复
    ERR_ENVIRONMENT = 1577213475
    # 校验ctl版本失败
    ERR_CHECK_CTL_VERSION = 1677873254
    # 跨v6.0.0版本异机恢复new_collation_enabled冲突
    ERR_CONFLICT_COLLATIONS = 1677873257


class ClusterRequiredHost:
    PD = "pd"
    TIDB = "tidb"
    TIKV = "tikv"
    TIFLASH = "tiflash"
    TIUP = "tiup"


class TiDBResourceKeyName:
    """
    Tidb资源接入用户key
    """
    APPLICATION_AUTH_AUTHKEY = "appEnv_auth_authKey_"
    APPLICATION_AUTH_AUTHPWD = "appEnv_auth_authPwd_"
    LIST_APPLICATION_AUTH_AUTHKEY = "applications_0_auth_authKey_"
    LIST_APPLICATION_AUTH_AUTHPWD = "applications_0_auth_authPwd_"
    APPLICATION_AUTH_AUTHKEY_BKP = "job_protectObject_auth_authKey_"
    APPLICATION_AUTH_AUTHPWD_BKP = "job_protectObject_auth_authPwd_"
    APPLICATION_AUTH_AUTHKEY_RST = "job_targetObject_auth_authKey_"
    APPLICATION_AUTH_AUTHPWD_RST = "job_targetObject_auth_authPwd_"


class TiDBConst:
    # 数据库常量
    ALL_HOSTS = "%"
    ALL_PRIVILEGE = "ALL PRIVILEGES"
    DROP = "DROP"
    SELECT = "SELECT"
    BR_VERSION = "Release Version"
    LOG_PATH = "storage"
    LOG_STATUS = "status"
    LOG_NORMAL = "NORMAL"
    CLUSTER_USER = "Deploy user"
    # meta 常量
    LOG_MIN_TS = "log-min-ts"
    LOG_MAX_TS = "log-max-ts"
    BACKUP_META = "backupmeta"
    # 时间
    SYSTEM = "system"
    START = "start"
    CHECKPOINT = "checkpoint[global]"
    # 节点信息常量
    PD_DOWN = "pd down"
    TIDB_DOWN = "tidb down"
    TIKV_DOWN = "tikv down"
    TIFLASH_DOWN = "tiflash down"
    # 报错回显相关常量
    DATABASE_EXIST = 'ErrDatabasesAlreadyExisted'
    DROP_DB_FAILED = "Drop databases on target cluster failed."
    LOG_TASK_EXIST = "Log task exist"
    USER_ID_CHECK_FAILED = "userid check failed"
    CONFLICT_TABLES = "exist conflict tables"
    ERROR_START = "Error:"
    CONFLICT_COLLATIONS = "'new_collations_enabled_on_first_bootstrap' not match"


class TiDBDataBaseFilter:
    # 集群下默认数据库，不需备份的数据库
    INFORMATION_SCHEMA = "INFORMATION_SCHEMA"
    METRICS_SCHEMA = "METRICS_SCHEMA"
    PERFORMANCE_SCHEMA = "PERFORMANCE_SCHEMA"
    MYSQL = "mysql"
    TIDB_BR_TEMP = "__TiDB_BR_Temporary_mysql"


class TidbPath:
    TIDB_FILESYSTEM_MOUNT_PATH = "/mnt/databackup/"
    TIDB_LINK_PATH = f"{get_install_head_path()}/DataBackup/"


class TidbSubJobName:
    SUB_DEPLOY_USER = "sub_deploy_user"
    SUB_CHECK_UP = "sub_check_up"
    SUB_RECORD_UID = "sub_record_uid"
    SUB_CHECK_UID = "sub_check_uid"
    SUB_CREATE = "sub_create"
    SUB_EXEC = "sub_exec"
    SUB_RECORD = "sub_record"
    SUB_UP_LOG = "sub_up_log"
    SUB_KF_LOG = "sub_kv_log"


class TdsqlBackupStatus:
    SUCCEED = "Completed"
    RUNNING = "Running"
    FAILED = "Failed"


class LastCopyType:
    last_copy_type_dict = {
        1: [RpcParamKey.FULL_COPY],
        2: [RpcParamKey.LOG_COPY],
        3: [RpcParamKey.FULL_COPY, RpcParamKey.LOG_COPY]
    }


class SqliteServiceField(Enum):
    SQLITE_DATABASE_NAME = 'copymetadata.sqlite'
    SQLITE_DATATABLE_NAME = 'T_COPY_METADATA'
    TYPE_CLUSTER = 'cluster'
    TYPE_DATABASE = 'database'
    TYPE_TABLE = 'table'


class MysqlTimeOut:
    MYSQL_TIME_OUT = 20


class TidbSubJobNames:
    SUB_JOBS = (
        TidbSubJobName.SUB_DEPLOY_USER, TidbSubJobName.SUB_CHECK_UP, TidbSubJobName.SUB_RECORD_UID,
        TidbSubJobName.SUB_CHECK_UID, TidbSubJobName.SUB_CREATE, TidbSubJobName.SUB_EXEC, TidbSubJobName.SUB_RECORD,
        TidbSubJobName.SUB_UP_LOG, TidbSubJobName.SUB_KF_LOG
    )


class TiupGeneralErrPatterns:
    RST_PATTERNS = {
        TiDBConst.PD_DOWN: "All pd hosts down. Please check target cluster status.",
        TiDBConst.TIDB_DOWN: "All tidb hosts down. Please check target cluster status.",
        TiDBConst.TIKV_DOWN: "Tikv host down. Please check target cluster status.",
        TiDBConst.TIFLASH_DOWN: "Tiflash host down. Please check target cluster status.",
        TiDBConst.DROP_DB_FAILED: TiDBConst.DROP_DB_FAILED,
        TiDBConst.LOG_TASK_EXIST:
            "Log task exists on target cluster! Stop log task and try again.",
        TiDBConst.DATABASE_EXIST: "[BR:Restore:ErrDatabasesAlreadyExisted] "
                                  "databases already existed in restored cluster.",
        TiDBConst.USER_ID_CHECK_FAILED: "Uid check failed. Please make sure target cluster Deploy user has "
                                        "the same uid on all tikv and tiflash hosts.",
        TiDBConst.CONFLICT_TABLES: "Conflicts tables exist in the target cluster."
    }


class TiupRecognizedErrPatterns:
    RST_PATTERNS = {
        TiDBConst.CONFLICT_COLLATIONS: ErrorCode.ERROR_AUTH
    }
