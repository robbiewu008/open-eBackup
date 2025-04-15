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
from common.const import SubJobStatusEnum, ReportDBLabel
from common.env_common import get_install_head_path


class MySQLType:
    TYPE = "Database"
    SUBTYPE = "MySQL-database"
    SUBTYPECLUSTER = "MySQL-clusterInstance"


class SystemServiceType:
    SYSTEMCTL = "systemctl"
    SERVICE = "service"


class MySQLClusterType:
    AP = "AP"
    PXC = "PXC"
    EAPP = "EAPP"
    AA = "AA"


class MySQLStrConstant:
    MYSQL = "mysql"
    MYSQLDSERVICES = "mysqld.service"
    MYSQLSERVICES = "mysql.service"
    EAPPMYSQLSERVICES = "eappmysql.service"
    MARIADBSERIVCE = "mariadb.service"
    MYSQLPXCSERVICES = "mysql@bootstrap.service"
    MYSQLD = "mysqld"
    MYSQLAPPLICTATION = "MySQL"
    MARIADB = "MariaDB"
    DEFAULT_BIN_LOG_PATTERN = "mysql-bin"
    SERVER_ID = "server-id"
    SKIP_SLAVE_START = "skip-slave-start"
    LD_LIBRARY_PATH = "LD_LIBRARY_PATH"
    LIB64 = "/lib64"
    UNDO_PREFIX = "undo"
    GAP_GTID = "_gap_gtid"
    GREATSQL = "greatsql"
    GREATSQLAPPLICTATION = "GreatSQL"
    GREATSQLSERVICE = "greatsql.service"


class SystemConstant:
    CENTOS_STREAM_9 = "CentOS Stream release 9"
    CHARSET_SUPPORT_LIST = ['utf8mb4', 'latin1']
    DEFAULT_CHARSET = "utf8mb4"
    DEFAULT_PORT = "3306"
    UNSUPPORTED_INSTANT_VERSION = ["8.0.29", "8.0.30", "8.0.31"]
    # 设置mysqlbinlog 最大恢复数据量，防止日志恢复失败,单位是字节
    MAX_ALLOWED_PACKET = 1024 * 1024 * 1024
    DEFAULT_LOCK_TIME = "00:00:01"


class MariaDBConstant:
    MARIADB = "MariaDB"


class MySQLJsonConstant:
    SYSTEM_SERVICE_TYPE_KEY = "systemServiceType"
    EXTENDINFO = "extendInfo"
    CLUSTERTYPE = "clusterType"
    JOB = "job"
    JOBPARAM = "jobParam"
    BACKUPTYPE = "backupType"
    PROTECTOBJECT = "protectObject"
    SUBTYPE = "subType"
    APPENV = "protectEnv"
    ENDPOINT = "endpoint"
    AUTH = "auth"
    AUTHKEY = "authKey"
    AUTHPWD = "authPwd"
    INSTANCEPORT = "instancePort"
    CHARSET = "charset"
    INSTANCEIP = "instanceIp"
    NODES = "nodes"
    REPORITTORIES = "repositories"
    REPORITORYTYPE = "repositoryType"
    PATH = "path"
    BACKJOBRESLUT = "backupJobResult"
    COPY = "copy"
    COPYID = "copyId"
    TIMESTAMP = "timestamp"
    BACKUPTIME = "backupTime"
    BACKUPLMITE = "BackupLimit"
    NAME = "name"
    COPIES = "copies"
    TARGETOBJECT = "targetObject"
    JOBPARAM = "jobParam"
    RESTOREJOB = "restoreType"
    BACKUPTYPE = "backupType"
    RESTOREJOBRESULT = "restoreJobResult"
    PROGRESS = "progress"
    TASKID = "taskId"
    SUBTASKID = "subTaskId"
    TASKSTATUS = "taskStatus"
    DATASIZE = "dataSize"
    SPEED = "speed"
    LOGDETAIL = "logDetail"
    LOGDETAILPARAM = "logDetailParam"
    APPLICTION = "application"
    JOBTYPE = "jobType"
    LIVEMOUNT = "liveMount"
    TARGETENV = "targetEnv"
    ROLE = "role"
    ADVANCEPARAMS = "advanceParams"
    SUBJOB = "subJob"
    JOBNAME = "jobName"
    RESTORETIMESTAMP = "restoreTimestamp"
    TYPE = "type"
    TYPES = "types"
    REMOTEHOST = "remoteHost"
    MOUNTJOBID = "mountJobId"
    LOGINFO = "logInfo"
    LOGINFOPARAM = "logInfoParam"
    LOGDETAILPARAM = "logDetailParam"
    LOGLEVEL = "logLevel"
    BACKUPTASKSLA = "backupTask_sla"
    POLICYLIST = "policy_list"
    EXTPARAMETERS = "ext_parameters"
    CHANNELNUMBER = "channel_number"
    NEW_DATABASE_NAME = "newDatabaseName"
    VERSION = "version"
    DATA_DIR = "dataDir"
    LOG_BIN_INDEX_PATH = "logBinIndexPath"
    AVERAGE_SPEED = "averageSpeed"
    LOG_FLAG = "logFlag"
    LOG_FLAG_START_TIME = "logFlagStartTime"
    RESTORE_COPY_ID = "restoreCopyId"
    SERVICE_NAME = "serviceName"
    ID = "id"
    BACKUPHOSTSN = "backupHostSN"
    BINLOG_NAMES = "backupBinlog"
    FIRSTFULLBACKUPTIME = "firstFullBackupTime"
    NEXTCAUSEPARAM = "next_cause_param"
    END_TIME = "endTime"
    FORCE_RECOVERY = "forceRecovery"
    FORBIDDEN_STRICT_MODE = "forbiddenStrictMode"
    MASTER_INFO = "master_info"
    AGENT_IP_LIST = "agentIpList"
    STORAGE_TYPE = "storage_type"
    ASSOCIATED_COPIES = "associatedCopies"


class IPConstant:
    LOCALHOST = "127.0.0.1"


class ForbiddenStrictModeOptions:
    ForbiddenStrictMode = 1
    AllowStrictMode = 0


class MySQLProgressFileType:
    COMMON = "common"
    ABORT = "abort"
    PROGRESSTIME = "progress_timestamp"


class ExecCmdResult:
    SUCCESS = "0"
    UNKNOWN_CMD = "127"


class MySQLExecPower:
    MYSQL_USER = "root"
    MYSQL_FILE_MODE = "0700"
    MYSQL_LIVEMOUNT_PATH = "/mnt"


class MySQLParamType:
    INSTANCE = 'MySQL-instance'
    DATABASE = 'MySQL-database'
    CLUSTER = 'MySQL-clusterInstance'


class RoleType:
    ACTIVE_NODE = "1"
    STANDBY_NODE = "2"


class RestoreSubJobType:
    ALLOW_RESTORE = "allow_restore"
    RESTORE = "restore"
    POST_RESTORE = "post"
    RESTART_CLUSTER = "restart_cluster"
    STOP_SLAVE = "stop_slave"
    SYNC_STATUS_CHECK = "sync_status_check"
    COMPARE_MASTER = "compare_master"
    RESET_SYNC = "reset_sync"


class MasterInfoFile(Enum):
    # master.info文件里某一行记录的类型
    HOST_IP_LINE = 3
    USER_LINE = 4
    PASSWORD_LINE = 5
    PORT_LINE = 6


class RestoreType:
    SINGLE_DB = "SingleDatabase"  # 单数据库
    AP_DB_CLUSTER_MASTER_NODE = "APDatabaseClusterMasterNode"  # 主备集群主节点数据库
    AP_DB_CLUSTER_SLAVE_NODE = "APDatabaseClusterSlaveNode"  # 主备集群备节点数据库
    PXC_DB_CLUSTER_NODE = "PXCDatabaseClusterNode"  # PXC集群集群节点数据库
    PXC_DB_COMMON_NODE = "PXCDatabaseCommonNode"  # PXC集群普通节点数据库
    EMPTY = "Empty"  # 其他类型
    SINGLE_INSTANCE = "SingleInstance"  # 单实例
    AP_INSTANCE_CLUSTER_MASTER_NODE = "APInstanceClusterMasterNode"  # 主备集群主节点实例
    AP_INSTANCE_CLUSTER_SLAVE_NODE = "APInstanceClusterSlaveNode"  # 主备集群备节点实例
    PXC_INSTANCE_CLUSTER_NODE = "PXCInstanceClusterNode"  # PXC集群集群节点实例
    PXC_INSTANCE_COMMON_NODE = "PXCInstanceCommonNode"  # PXC集群普通节点实例
    EAPP_INSTANCE = "EAPPInstance"


class RestorePath:
    MYSQL_TMP_PATH = "/mnt"
    MYSQL_RESTORE_PATH = "restore"
    DATADIR = "datadir"
    DOTOLD = ".old"
    HOST_SN_FILE_PATH = "/etc/HostSN/HostSN"
    SERVER_ID_FILE = "server-id"


class MySQLCmdStr:
    ALLOW_BACKUP_IN_LOCAL = "AllowBackupInLocalNode"
    CHECK_BACKUP_JOB_TYPE = "CheckBackupJobtype"
    BACKUP_PER = "BackupPerrequisite"
    BACKUP = "Backup"
    BACKUP_POST = "BackupPostJob"
    QUERY_COPY = "QueryBackupCopy"
    QUERY_PERMMISSION = "QueryJobPermission"
    PROGRESS_LIVE_MOUNT = "progress_livemMount"
    PROGRESS_COMM = "progress_comm"
    PROGRESS_ABORT = "progress_abort"
    ABORT_JOB = "AbortJob"
    PAUSE_JOB = "PauseJob"
    LIVE_MOUNT = "LiveMount"
    CANCEL_LIVE_MOUNT = "CancelLiveMount"
    RESTORE_PRE = "RestorePrerequisite"
    RESTORE = "Restore"
    RESTORE_POST = "RestorePostJob"
    RESTORE_GEN_SUB = "restore_gen_sub"


class MySQLRestoreStep:
    RENAME_BEFORE_RESTORE = 1  # 恢复前文件或目录改名
    OPERATE_NON_COMMON_NODE = 2  # 恢复前非PXC集群普通节点的操作
    XTRABACKUP_RESTORE = 3  # 恢复命令执行
    CHOWN = 4  # 权限修改
    RESTART = 5  # 重启节点数据库
    STOP_SLAVE = 6  # AP集群备节点的slave状态操作
    LOG_RESTORE = 7  # 日志恢复
    OPERATE_MASTER_NODE = 8  # AP集群主节点记录主状态的当前文件和位置信息


class MysqlLabel:
    BACKUP_LOCK_TABLE_DETAIL = 'mysql_plugin_backup_lock_detail_label'
    label_dict = {
        MySQLCmdStr.BACKUP: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.BACKUP_SUB_FAILED,
            SubJobStatusEnum.RUNNING.value: ReportDBLabel.BACKUP_SUB_START_COPY,
            SubJobStatusEnum.COMPLETED.value: BACKUP_LOCK_TABLE_DETAIL
        },
        MySQLCmdStr.BACKUP_POST: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.BACKUP_SUB_FAILED
        },
        MySQLCmdStr.RESTORE: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.RESTORE_SUB_FAILED,
            SubJobStatusEnum.RUNNING.value: ReportDBLabel.RESTORE_SUB_START_COPY
        },
        MySQLCmdStr.RESTORE_POST: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.RESTORE_SUB_FAILED
        },
        MySQLCmdStr.LIVE_MOUNT: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.LIVE_MOUNT_FAILED
        },
        MySQLCmdStr.CANCEL_LIVE_MOUNT: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.CANCEL_LIVE_MOUNT_FAILED
        },
        MySQLCmdStr.BACKUP_PER: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.PRE_REQUISIT_FAILED
        },
        MySQLCmdStr.RESTORE_PRE: {
            SubJobStatusEnum.FAILED.value: ReportDBLabel.PRE_REQUISIT_FAILED
        }
    }


class MysqlParentPath:
    TMP = "/mnt/"
    TMPOCEANPROTECT = "/mnt/databackup/"


class MysqlBackupToolName:
    XTRBACKUP2 = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/xtrabackup2"
    XTRBACKUP8 = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/xtrabackup8"
    MARIADBBACKUP = "mariabackup"
    MYSQLBINLOG = "mysqlbinlog"
    MYSQLDUMP = "mysqldump"
    MYSQL = "mysql"
    MYSQLD = "mysqld"
    MYSQLADMIN = "mysqladmin"
    MARIADB = "MariaDB"


class MySQLConfigPath:
    CONFIGPATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf/MySQL_LOCK.conf"


class MySQLPreTableLockStatus:
    ON = '1'
    OFF = '0'


class MysqlProgress:
    ONE_HUNDRED = 100
    FIVE = 5


class FileBackToolRET:
    SUCCESS = 1
    RUNNING = 2
    FAILED = 3


class MysqlExecSqlError:
    ERROR_TABLE_NOT_FOUND = "(1146,"
    ERROR_ACCESS_DENINED = "(1045,"


class MysqlStatusStr:
    STOP = "Active: inactive (dead)"
    FAILED = "Active: failed"


class SubJobType(int, Enum):
    PRE_SUB_JOB = 0,
    GENERATE_SUB_JOB = 1,
    BUSINESS_SUB_JOB = 2,
    POST_SUB_JOB = 3


class SubJobPolicy(int, Enum):
    LOCAL_NODE = 1,
    EVERY_NODE_ONE_TIME = 2,
    RETRY_OTHER_NODE_WHEN_FAILED = 3,
    FIXED_NODE = 4


class SubTaskPriority(int, Enum):
    ACTIVE_NODE_INI_PRIORITY = 1,
    STANDBY_NODE_INIT_PRIORITY = 2,
    MAX_PRIORITY_PLUS_VALUE = 3


class MariaDBNeedExcludeDir:
    # MariaDB做数据库恢复时，需要排除的目录
    ROCKSDB = "#rocksdb"
    HASGTAG = "#"


class MysqlPxcStrictMode:
    DISABLED = "DISABLED"
    PERMISSIVE = "PERMISSIVE"
    ENFORCING = "ENFORCING"
    MASTER = "MASTER"


DELETING_PATH_WHITE_LIST = [
    "/mnt/databackup/",
    f"{get_install_head_path()}/DataBackup/",
    "/mnt/"
]

# 操作系统列表 Other为其他操作系统，未在规格中。
DEPLOY_OPERATING_SYSTEMS = (
    "Other",
    "Red Hat",
    "CentOS",
    "SUSE",
    "euleros"
)

DELETING_PATH_BLACK_LIST = r"^/$|^/bin$|^/boot$|^/dev$|^/etc$|^/lib$|^/lib64$|^/lost+found$|^/media$|^/mnt$|" \
                           "^/proc$|^/root$|^/run$|^/sbin$|^/selinux$|^/srv$|^/sys$|^/usr$|^/usr/bin$|" \
                           "^/usr/include$|^/usr/lib$|^/usr/local$|^/usr/local/bin$|^/usr/local/include$|" \
                           "^/usr/local/sbin$|^/usr/local/share$|^/usr/sbin$|^/usr/share$|^/usr/src$|^/var$|^/data$| " \
                           "^/bin/$|^/boot/$|^/dev/$|^/etc/$|^/lib/$|^/lib64/$|^/lost+found/$|^/media/$|^/mnt/$|" \
                           "^/proc/$|^/root/$|^/run/$|^/sbin/$|^/selinux/$|^/srv/$|^/sys/$|^/usr/$|^/usr/bin/$|" \
                           "^/usr/include/$|^/usr/lib/$|^/usr/local/$|^/usr/local/bin/$|^/usr/local/include/$|" \
                           "^/usr/local/sbin/$|^/usr/local/share/$|^/usr/sbin/$|^/usr/share/$|^/usr/src/$|^/var/$|" \
                           "^/data/$|^/opt/$|^/opt$"

MYSQL_SERVICE_LIST = [
    MySQLStrConstant.MYSQLDSERVICES,
    MySQLStrConstant.EAPPMYSQLSERVICES,
    MySQLStrConstant.MYSQLSERVICES,
    MySQLStrConstant.MARIADBSERIVCE,
    MySQLStrConstant.MYSQLPXCSERVICES,
    MySQLStrConstant.MYSQLD,
    MySQLStrConstant.MYSQL,
    MySQLStrConstant.GREATSQL,
    MySQLStrConstant.GREATSQLSERVICE
]


class XtrbackupErrStr:
    # 由于系统表空间中的数据库页已损坏
    CORRUPT_DATABASE_PAGE = "Aborting because of a corrupt database page in the system tablespace"
    # 数据库副本数据已经准备好恢复了
    DATABASE_COPY_IS_PREPARED = "This target seems to be already prepared"


class MysqlPrivilege:
    # 所有权限
    ALL_PRIVILEGES = "ALL PRIVILEGES"

    # MySQL权限字典
    MYSQL_VERSION_PRIVILEGE = {
        "5": ["RELOAD", "PROCESS", "LOCK TABLES"],
        "8": ["BACKUP_ADMIN", "PROCESS", "RELOAD", "SELECT", "LOCK TABLES"]
    }

    # MariaDB权限字典
    MARIADB_VERSION_PRIVILEGE = {
        "5.5":  ["RELOAD", "PROCESS", "SUPER", "REPLICATION CLIENT", "LOCK TABLES"],
        "10.2": ["RELOAD", "PROCESS", "SUPER", "REPLICATION CLIENT", "LOCK TABLES"],
        "10.3": ["RELOAD", "PROCESS", "LOCK TABLES"],
        "10.4": ["RELOAD", "PROCESS", "LOCK TABLES"],
        "10.5": ["RELOAD", "PROCESS", "LOCK TABLES"],
        "10.6": ["RELOAD", "PROCESS", "LOCK TABLES"],
        "10.7": ["RELOAD", "PROCESS", "LOCK TABLES"],
    }

    @staticmethod
    def is_mariadb(version: str):
        return MySQLStrConstant.MARIADB in version

    # MySQL 5.x MariaDB 10.3, 10.4 权限二选一必选
    MYSQL_OPTIONAL_PRIVILEGES = ["SUPER", "REPLICATION CLIENT"]

    # MARIADB 10.5（10.6、10.7）权限二选一必选
    MARIADB_OPTIONAL_PRIVILEGES = ["SUPER", "BINLOG MONITOR"]

    # 权限IP集合
    PRIVILEGE_IPS = ["%", "localhost", "127.0.0.1"]

    MARIADB_OPTIONAL_MAP = {
        "5.5": [],
        "10.2": [],
        "10.3": ["SUPER", "REPLICATION CLIENT"],
        "10.4": ["SUPER", "REPLICATION CLIENT"],
        "10.5": ["SUPER", "BINLOG MONITOR"],
        "10.6": ["SUPER", "BINLOG MONITOR"],
        "10.7": ["SUPER", "BINLOG MONITOR"],
    }

    @staticmethod
    def get_optional_privilege(version):
        # mysql的可选权限
        if not MysqlPrivilege.is_mariadb(version):
            if str(version).startswith("8"):
                return []
            return MysqlPrivilege.MYSQL_OPTIONAL_PRIVILEGES
        for maria_version in MysqlPrivilege.MARIADB_OPTIONAL_MAP.keys():
            if str(version).startswith(maria_version):
                return MysqlPrivilege.MARIADB_OPTIONAL_MAP.get(maria_version)
        return []

    @staticmethod
    def get_necessary_privilege(version):
        if not MysqlPrivilege.is_mariadb(version):
            return MysqlPrivilege.MYSQL_VERSION_PRIVILEGE.get(version[0:1])
        for maria_version in MysqlPrivilege.MARIADB_VERSION_PRIVILEGE.keys():
            if str(version).startswith(maria_version):
                return MysqlPrivilege.MARIADB_VERSION_PRIVILEGE.get(maria_version)
        return MysqlPrivilege.MARIADB_VERSION_PRIVILEGE.get("10.7")


class SubJobName(str, Enum):
    FLUSH_LOG = "flush_log"
    BACKUP = "backup_job"
    REPORT_COPY = "write_copy"


# mysql配置文件里的每个参数的key，中横线和下横线都是一样的效果
class MysqlConfigFileKey:
    INNODB_TABLE_SPACE = ["innodb_undo_tablespaces"]
    BASE_DIR = ["basedir"]
    DATA_DIR = ["datadir"]
    LOG_BIN_INDEX_ARR = ["log_bin_index", "log-bin-index"]
    LOG_BIN_ARR = ["log_bin", "log-bin"]
    INNODB_UNDO_DIRECTORY = ["innodb_undo_directory"]
    RELAY_LOG_ARR = ["relay-log", "relay_log"]
    RELAY_LOG_DIRECTORY = ["relay_log_recovery"]
    EXTRA_CONFIG = ["lower_case_table_names"]


class SQLCMD:
    select_instant = "select name from information_schema.innodb_tables where total_row_versions > 0"
    flush_logs = "flush binary logs"
    show_tables = "show tables"
