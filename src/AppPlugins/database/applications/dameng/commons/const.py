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


class BackupStepEnum(str, Enum):
    ALLOW_BACKUP_IN_LOCAL_NODE = "AllowBackupInLocalNode"
    CHECK_BACKUP_JOB_TYPE = "CheckBackupJobType"
    PRE_TASK = "BackupPrerequisite"
    PRE_PROGRESS = "PrerequisiteProgress"
    BACKUP = "Backup"
    BACKUP_PROGRESS = "BackupProgress"
    POST_TASK = "BackupPostJob"
    POST_TASK_PROGRESS = "BackupPostJobProgress"
    STOP_TASK = "AbortJob"
    STOP_TASK_PROGRESS = "AbortJobProgress"
    QUERY_BACKUP_COPY = "QueryBackupCopy"
    GENERATOR_SUB_JOB = "GeneratorSubJob"
    CHECK_APPLICATION = "CheckApplication"
    DAMANG_CLUSTER = "DamengCluster"
    QUERY_SCAN_REPOSITORIES = "QueryScanRepositories"


class BackupSubType(str, Enum):
    SINGLE_NODE = "Dameng-singleNode"
    CLUSTER = "Dameng-cluster"


class ResourcesSubType(str, Enum):
    CHECKAPPLICATION = "CheckApplication"
    SINGLE_NODE = "Dameng-singleNode"
    CLUSTER = "Dameng-cluster"
    JOBPERMISSON = "QueryJobPermission"


class DbStatus(str, Enum):
    DB_STATUS_OPEN = "OPEN"
    DB_STATUS_MOUNT = "MOUNT"
    DB_STATUS_CLOSE = "CLOSE"


class DbArchStatus(str, Enum):
    OPEN_ARCH = 'Y'
    CLOSE_ARCH = 'N'


class DbAuthInfo(str, Enum):
    RIGHT = "right"
    ERRORS = "errors"


class ClusterNodeMode(str, Enum):
    NODE_MODE_NORMAL = "NORMAL"
    NODE_MODE_PRIMARY = "PRIMARY"
    NODE_MODE_STANDBY = "STANDBY"


class DamengStrConstant:
    CLUSTER_BACKUP_BASE_DIR = "/mnt/databackup/Dameng"
    BACKUP_TYPE_FULL = "full"
    BACKUP_TYPE_INCREMENT = "increment"
    BACKUP_TYPE_ARCHIVE = "archive"
    HOSTSN_FILE_PATH = "/etc/HostSN/HostSN"
    ENV_DM_HOME = "DM_HOME"
    KEY_PREFIX = "job_protectEnv"
    SHELL_TYPE_SH = "/bin/sh"
    SHELL_TYPE_BASH = "/bin/bash"
    PWD_FILE = "/etc/passwd"
    DM_CONFIG_FILE = "/etc/dm_svc.conf"
    USER_ROOT = "root"
    SCRIPT_INSTALLER_USER = "dm_service_installer.sh"
    DM_INIT = "dminit"
    DM_WATCHER = "DmWatcher"
    DM_ARCH_DEST = "ARCH_DEST"
    DIVIDED_BY_2 = 2
    ONE_HUNDRED = 100
    ENV_LANG = "LANG"


class DamengStrFormat:
    DMCTLCVT = 'su - {} -c \"{}/dmctlcvt TYPE={} SRC={} DEST={}\"'
    DMRMAN = "su - {} -c \"{}/dmrman CTLFILE=\'{}\'\""
    DMRMAN_REDIRECT_OUTPUT = "su - {} -c \"{}/dmrman CTLFILE=\'{}\' 1>{} 2>&1\""
    DISQL_LOGIN_PASSWORD = "su - {} -s /bin/bash -c \"{}/disql /nolog\""
    DATABASE_LOGIN = "conn {}/{}@127.0.0.1:{}"
    DISQL_LOGIN_OS = "su - {} -c \"{}/disql /\@{}:{} as SYSDBA\""
    DISQL_LOGIN_OS_WITH_MPP_TYPE = "su - {} -c \"{}/disql /\@{}:{}#\"{{mpp_type=local}}\" as SYSDBA\""
    SHOW_BACKUPSET = "show backupset \'{}\' to \'{}\' format xml ;"
    SU_CMD = "su - {} -s {} -c '{}'"
    CAT_SERVICE_GET_DMINI = 'su - {} -c "cat {}/{} |grep INI_PATH"'
    MOUNT_BIND = "mount --bind {} {}"
    DB_RESTART_CMD = 'su - {} -c "{} restart"'
    CHECK_DMWATCH_RUNNING_CMD = 'su - {} -c "{} status"'
    STOP_DMWATCH_CMD = 'su - {} -c "{} stop"'
    START_DMWATCH_CMD = 'su - {} -c "{} start"'
    INCREMENT_BUCKUP_SQL = "BACKUP DATABASE INCREMENT BASE ON " \
                           "BACKUPSET \'{}\' BACKUPSET \'{}\' PARALLEL {};"
    CUMULATIVE_BUCKUP_SQL = "BACKUP DATABASE INCREMENT CUMULATIVE BASE ON " \
                            "BACKUPSET '{}' BACKUPSET '{}' PARALLEL {};"


class CheckBackupJobTypeCode:
    CAN_EXEC = 0
    INC_TO_FULL = 1577209901
    PARAM_ERROR = 1593987330


class ArrayIndex(int, Enum):
    INDEX_LAST_4 = -4
    INDEX_LAST_3 = -3
    INDEX_LAST_2 = -2
    INDEX_LAST_1 = -1
    INDEX_FIRST_0 = 0
    INDEX_FIRST_1 = 1
    INDEX_FIRST_2 = 2
    INDEX_FIRST_3 = 3
    INDEX_FIRST_4 = 4
    INDEX_FIRST_5 = 5


class RetoreCmd:
    RESTORE_PRE = "restore_pre"
    RESTORE_GEN_SUB = "restore_gen_sub"
    RESTORE_EXEC_SUB = "restore_exec_sub"
    RESTORE_POST = "restore_post"
    RESTORE_PROGRESS = "restore_progress"
    RESTORE_ALLOW = "restore_allow"


class ProgressType:
    PRE = "pre"
    RESTORE_SUB = "restore_sub"
    POST = "post"


class DMFunKey:
    INIT = "init_param"
    EXEC_TASK = "exec_task"
    EXEC_EXCEPT = "exec_cmd_when_execpt"
    EXEC_SUCCESS = "exec_cmd_when_success"
    EXEC_FAILED = "exec_cmd_when_failed"


class DMJsonConstant:
    EXTENDINFO = "extendInfo"
    CLUSTERTYPE = "clusterType"
    JOB = "job"
    SUBJOB = "subJob"
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
    NODES = "nodes"
    REPORITTORIES = "repositories"
    REPORITORYTYPE = "repositoryType"
    REMOTEPATH = "remotePath"
    PATH = "path"
    BACKJOBRESLUT = "backupJobResult"
    COPY = "copy"
    TIMESTAMP = "timestamp"
    BACKUPTIME = "backupTime"
    BACKUPLMITE = "BackupLimit"
    NAME = "name"
    COPIES = "copies"
    TARGETOBJECT = "targetObject"
    TARGETENV = "targetEnv"
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
    LOGDETAIL = "log_detail"
    APPLICTION = "application"
    JOBTYPE = "jobType"
    DBNAME = "dbName"
    DBPATH = "dbPath"
    DBPORT = "dbPort"
    DMINIPATH = 'dminiPath'
    BACKUPSETNAME = "backupSetName"
    BACKUPTYPE = "backupType"
    BASEBACKUPSETNAME = "baseBackupSetName"
    ID = "uuid"
    ROLE = "role"
    PORT = "port"
    GROUPID = "groupId"
    GROUPINDEX = "groupIndex"
    JOBINFO = "jobInfo"
    TARGETLOCATION = "targetLocation"
    ORIGINAL = "original"
    NEW = "new"
    JOBNAME = "jobName"
    NODEINDEX = "nodeIndex"
    ROLETYPE = "roleType"
    TABALSPACE = "tableSpace"
    VERSION = "version"
    RESTORESUBOBJECTS = "restoreSubObjects"
    RESTORESCN = "restoreScn"
    RESTORETIMESTAMP = "restoreTimestamp"
    TABAL_SPACE_INFO = "tabal_space_info"
    TABLE_LEN = "tabal_len"
    TABLE_NAME = "tabal_name"
    B_TO_KB = 1024
    DATAPATH = "dataPath"


class DmRestoreType:
    TABALSPACE = "tablespace"
    TIMESTAMP = "timestamp"
    DATABASE = "database"
    SCN = "restorescn"


class ExecCmdResult:
    SUCCESS = "0"
    UNKNOWN_CMD = "127"


class ConfigureFlag:
    NOT_EXIST = "0"
    EXIST = "1"


class DMRestoreSubjobName:
    RESTORE = "dm_restore"
    START = "dm_start"
    UNMOUNT = "dm_unmount"
    GENERARESTORE = "restore"


class DMArchIniStr:
    ARCHIVE_LOCAL = "ARCHIVE_LOCAL1"
    ARCH_TYPE = "ARCH_TYPE"
    ARCH_TYPE_VALUE = "LOCAL"
    ARCH_DEST = "ARCH_DEST"
    ARCH_FILE_SIZE = "ARCH_FILE_SIZE"
    ARCH_FILE_SIZE_VALUE = "128"
    ARCH_SPACE_LIMIT = "ARCH_SPACE_LIMIT"
    ARCH_SPACE_LIMIT_VALUE = "0"
    ARCH_FLUSH_BUF_SIZE = "ARCH_FLUSH_BUF_SIZE"
    ARCH_FLUSH_BUF_SIZE_VALUE = "0"
    ARCH_HANG_FLAG = "ARCH_HANG_FLAG"
    ARCH_HANG_FLAG_VALUE = "1"


class ErrCode:
    AUTH_INFO_ERR = 1577209994
    DB_ARCH_NOT_OPEN = 1577209998
    DB_NOT_RUNNING = 1577084161
    DMAPSERVER_NOT_RUNNING = 1577209999
    DAMENG_DBNAME_CONFLICT = 1577210073
    TABLESPACE_NOT_EXISTS = 1577210077
    ERR_BACKUP_HISTORY = 1577210094
    ERR_EXECUTE_SQL_FAIL = 1577210103
    ERR_DB_IS_RUNNING = 1577210048
    ERR_DB_INIT_FAIL = 1577210107
    ERR_EXECUTE_RMAN_FAIL = 1577209860
    ERR_DB_AUTH_INSUFFICIENT_PERMISSION = 1577213465
    ERR_NODES_DISTRIBUTION_EXCEPTION = 1577213444
    PERMISSION_ERROR = 1577210111
    ERR_INCONSISTENT_CLUSTER_TOPOLOGY = 1577209866
    ERR_INVALID_LOG_COPY = 1577209918
    EXEC_BACKUP_RECOVER_CMD_FAIL = 1577209989
    INSTANCE_REGISTER_ERROR = 1577213550


class Secret:
    empty = ''


class RestoreProgress:
    start_progress = 0
    half_progress = 50
    completed_progress = 99
    failed_progress = 100


class SysData:
    SYS_STDIN = ""


class JobProgressLabel:
    backup_subjob_success = "dameng_plugin_backup_subjob_success_label"
    backup_subjob_running = "dameng_plugin_backup_subjob_running_label"


class LastCopyType:
    last_copy_type_dict = {
        1: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY, RpcParamKey.DIFF_COPY, RpcParamKey.LOG_COPY],
        2: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY],
        3: [RpcParamKey.FULL_COPY],
        4: [RpcParamKey.LOG_COPY]
    }


MAX_LSN = 9223372036854775807

DELETING_PATH_WHITE_LIST = [
    "/mnt/databackup/",
    f"{get_install_head_path()}/DataBackup/data/",
    f"{get_install_head_path()}/DataBackup/log/",
    f"{get_install_head_path()}/DataBackup/DmAgentcache/"
]

BACKUP_MOUNT_PATH = f"{get_install_head_path()}/DataBackup/data"

LOG_MOUNT_PATH = f"{get_install_head_path()}/DataBackup/log"

DM_FILE_PATH = f"{get_install_head_path()}/DataBackup/DmAgentcache/cache"

ARCH_FLAG = '1'

MPP_FLAG = '1'
