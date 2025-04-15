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

from common.logger import Logger
from common.env_common import get_install_head_path


class BackupMode:
    FULL = "FULL"
    PTRACK = "PTRACK"


class JobType:
    QUERY_JOB_PERMISSION = "QueryJobPermission"
    ALLOW_BACKUP_IN_LOCAL_NODE = "AllowBackupInLocalNode"
    CHECK_BACKUP_TYPE = "CheckBackupJobType"
    PREREQUISITE = "BackupPrerequisite"
    BACKUPGENSUB = "BackupGenSubJob"
    BACKUP = "Backup"
    POST = "BackupPostJob"
    ASYNC_ABORT = "AbortJob"
    PAUSE = "PauseJob"
    PREREQUISITE_PROGRESS = "PrerequisiteProgress"
    BACKUP_PROGRESS = "BackupProgress"
    POST_PROGRESS = "PostProgress"
    QUERY_BACKUP_COPY = "QueryBackupCopy"
    QUERY_SCAN_REOSITORIES = "QueryScanRepositories"
    ALLOW_RESTORE_IN_LOCAL_NODE = "AllowRestoreInLocalNode"
    RESTORE_PREREQUISITE = "RestorePrerequisite"
    RESTOREGENSUB = "RestoreGenSubJob"
    RESTORE = "Restore"
    RESTORE_POST = "RestorePost"
    RESTORE_PROGRESS = "RestoreProgress"


class SubJobPolicy(int, Enum):
    ANY_NODE = 0
    LOCAL_NODE = 1,
    EVERY_NODE_ONE_TIME = 2,
    RETRY_OTHER_NODE_WHEN_FAILED = 3,
    FIXED_NODE = 4


class OpenGaussSubJobName:
    EMPTY_SUB_JOB = "empty_sub_job"
    SUB_EXEC = "sub_exec"
    QUERY_COPY = "queryCopy"


class OpenGaussRestoreSubJobName:
    SUB_EXEC = "sub_exec"
    QUERY_COPY = "queryCopy"


class ResultCode:
    SUCCESS = "0"
    FAILED = "1"


class Status:
    NORMAL = "Normal"
    UNAVAILABLE = "Unavailable"
    DEGRADED = "Degraded"


class ReplicationMode:
    ASYNC = "Async"
    SYNC = "Sync"


class ParamKey:
    JOB = "job"
    JOB_ID = "jobId"
    REPOSITORIES = "repositories"
    AUTH = "auth"
    PATH = "path"
    INSTANCE = "instance"
    ENDPOINT = "endpoint"
    JOB_PARAM = "jobParam"
    BACKUP_TYPE = "backupType"
    PROTECT_OBJECT = "protectObject"
    TYPE = "type"
    SUB_TYPE = "subType"
    DATABASE = "database"
    PORT = "port"
    NAME = "name"
    ID = "id"
    REPOSITORY_TYPE = "repositoryType"
    ENV_FILE = "envPath"
    CLUSTER_VERSION = "clusterVersion"
    PROTECT_ENV = "protectEnv"
    EXTEND_INFO = "extendInfo"
    COPY = "copy"
    COPYIES = "copies"
    TIMESTAMP = "timestamp"
    PARENT_NAME = "parentName"
    TARGET_OBJECT = "targetObject"
    TARGET_ENV = "targetEnv"
    NEW_NAME = "newName"
    EXTEND_AUTH = "extendAuth"
    ESN_ID = "esnId"
    REMOTE_PATH = "remotePath"
    REMOTE_HOST = "remoteHost"
    IP = "ip"
    DATA = "data"
    DCS_ADDRESS = "dcsAddress"
    DCS_PORT = "dcsPort"
    DCS_USER = "dcsUser"
    CHANNEL_NUMBER = "channel_number"
    DEPLOY_TYPE = "deployType"
    MINUTE = 60
    HOUR = 60
    DAY = 24
    HALF_HOUR = 30


class MetaDataKey:
    FILE_NAME_SUFFIX = ".meta"
    COPY_ID = "copyId"
    COPY_TIME = "copyTime"
    COPY_FORMAT = "CopyFormatType"
    BACKUP_INDEX_ID = "backupIndexId"
    BACKUP_TYPE = "backupType"
    CLUSTER = "cluster"
    UUID = "uuid"
    NODES = "nodes"
    PROTECT_OBJECT = "protectObject"
    ID = "id"
    PARENT_COPY_ID = "parentCopyId"
    REPOSITORY_TYPE = "repositoryType"
    PATH = "path"
    TYPE = "type"
    SUB_TYPE = "subType"
    SUB_ID = "subId"
    ENDPOINT = "endpoint"
    EXTEND_INFO = "extendInfo"
    PROTECT_SIZE = "protectSize"
    PROTECT_NAME = "protectName"
    PG_PROBACKUP_CONF = "pg_probackup.conf"
    BASE_COPY_ID = "baseCopyId"
    USER_NAME = "userName"
    ENABLE_CBM_TRACKING = "enable_cbm_tracking"
    BEGIN_TIME = "begin_time"
    END_TIME = "end_time"
    BACKUP_TIME = "backup_time"


class CopyInfoKey:
    BACKUP_INSTANCE = "backup_instance"
    INSTANCE = "instance"
    BACKUPS = "backups"
    PARENT_BACKUP_ID = "parent-backup-id"
    UTC_TIME_SUFFIX = "+08"
    STATUS = "status"
    END_TIME = "end-time"
    START_LSN = "start-lsn"
    PG_PROBACKUP_CONF = "pg_probackup.conf"
    BACKUP_CONTROL = "backup.control"
    BACKUP_CONTENT_CONTROL = "backup_content.control"
    WAl = "wal"
    RECOVERY_TIME = "recovery-time"
    NO_TIME = "no_time"


class ProtectObject:
    OPENGAUSS = "openGauss"
    VASTBASE = "Vastbase"
    MOGDB = "MogDB"
    CMDB = "PanWeiDB"


class OpenGaussDeployType:
    SINGLE = "1"
    SHARDING = "3"
    DISTRIBUTED = "4"


class ProtectSubObject:
    INSTANCE = "OpenGauss-instance"
    DATABASE = "OpenGauss-database"


class Tool:
    GS_PROBACKUP = "gs_probackup"
    GS_DUMP = "gs_dump"
    GSQL = "gsql"
    GS_GUC = "gs_guc"
    GS_CTL = "gs_ctl"
    VB_PROBACKUP = "vb_probackup"
    VB_DUMP = "vb_dump"
    VSQL = "vsql"
    VB_GUC = "vb_guc"
    VB_CTL = "vb_ctl"
    CM_PROBACKUP = "pw_probackup"
    CM_DUMP = "pw_dump"
    CM_GUC = "pw_guc"
    CM_CTL = "pw_ctl"


class BackupSpeed:
    FILE_PREFFIX = "backupSpeed"
    START_TIME = "start_time"
    END_TIME = "end_time"


class SubApplication:
    OPENGAUSS = "openGauss"
    MOGDB = "MogDB"
    VASTBASE = "Vastbase"
    CMDB = "PanWeiDB"
    DISTRIBUTED = "distributed"


class SyncMode(int, Enum):
    SINGER = 0
    SYNC = 1
    ASYNC = 2


class OpenGaussType:
    TYPE = "Database"
    SUBTYPE = "OpenGauss"


class ProgressPercentage(int, Enum):
    START_PROGRESS = 0
    COMPLETE_PROGRESS = 100
    INCREMENT_BACKUP_PERCENTAGE = 50


class BackupFileCount(int, Enum):
    ZERO_FILE = 0
    ONE_FILE = 1


class CopyDirectory:
    INSTANCE_DIRECTORY = "instance_directory"
    DATABASE_DIRECTORY = "database_directory"


class Env:
    OPEN_GAUSS_USER = "application_auth_authKey"


class SubJobType(str, Enum):
    EMPTY = "empty"
    PREPARE_RESTORE = "prepare_restore"
    RESTORE = "restore"
    END_TASK = "endtask"
    RESTART = "restart"
    CMDB_RESTORE = "cmdb_restore"


class NodeRole:
    PRIMARY = "Primary"
    STANDBY = "Standby"


class AuthKey:
    APP_ENV = "appEnv_auth_authKey_"
    PROBECT_ENV = "job_protectEnv_auth_authKey_"
    APPLICATION_ENV = "application_auth_authKey_"
    TARGET_ENV = "job_targetEnv_auth_authKey_"
    PROTECT_ENV_DCS = "job_protectEnv_auth_extendInfo_dcsPassword_"
    TARGET_ENV_DCS = "job_targetEnv_auth_extendInfo_dcsPassword_"


class NodeDetailRole:
    PRIMARY = "P"
    STANDBY = "S"
    SINGLE = "single"
    NOTHING = "N"
    PRIMARY_NUM = "1"
    STANDBY_NUM = "2"


class RpcParamKey:
    APPLICATION = "application"
    TYPES = "types"
    FULL_COPY = "full"
    INCREMENT_COPY = "increment"
    COPY_ID = "copyId"
    INPUT_FILE_PREFFIX = "rpcInput"
    OUTPUT_FILE_PREFFIX = "rpcOutput"
    QUERY_PREVIOUS_CPOY = "QueryPreviousCopy"
    RPC_TOOL = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/rpctool.sh"


OPEN_GAUSS_USER = "application_auth_authKey"
APPENV = "appEnv"
APPLICATION = "application"
EVEN_PATH = "envPath"
EXTEND_INFO = "extendInfo"
AUTH = "auth"
AUTHKEY = "authKey"
CLUSTER_VERSION = "clusterVersion"
DEPLOY_TYPE = "deployType"
GUI_NODES = "guiNodes"
DCS_PASSWORD = "application_auth_extendInfo_dcsPassword"

SOURCE_RESULT = {
    'uuid': '', 'name': '', 'type': 'DataBase', 'subType': 'openGauss', 'endpoint': '',
    'nodes': [], 'extendInfo': {}
}

BASE_RET = {'code': 200, 'bodyErr': 0, 'message': ''}

CLUSTER_FIELD = (
    "node_name", "instance_port", "data_path", "node_ip", "instance_id",
    "instance_state", "instance_role", "type", "receiver_replay_location"
)

NODE_FIELD = ("nodeName", "datanodePort", "datanodeLocalDataPath", "datanodeListenIP 1")

INNER_DB = ("template0", "template1", "postgres", "vastbase")

conf_file = ["postgresql.conf", "pg_hba.conf", "pg_ident.conf", "gs_gazelle.conf", "mot.conf"]

DELETE_FILE_NAMES_OF_DATA_DIR = (
    "postmaster.pid", "postmaster.opts", "recovery.conf", "recovery.done",
    "recovery.signal", "backup_label.old"
)

SUCCESS_RET = 0

VB_DEFAULT_PORT = 5432

MAX_FILE_NUMBER_OF_LOG_BACKUP = 1000


class ProgressInfo(str, Enum):
    START = "Start to do restore"
    FAILED = "Failed to do task, unique id: RUO9LKH"
    SUCCEED = "Succeed to do task, unique id: RUO9LKH"


class FunctionResult:
    SUCCESS = 0
    FAILED = 1
    TRACKING_ERR = 2
    DEGRADE_ERR = 3
    ARCHIVEMODE_ERR = 4
    ARCHIVEDIR_ERR = 5


class TableSpace:
    TABLESPACE_PARENT_DIR = "pg_tblspc"


class GsprobackupParam:
    DEFAULT_PARALLEL = 16


logger = Logger().get_logger("openGauss-plugin.log")


class DatabaseToolLog:
    CONNECT_FAILED = "connect_failed"
    BACKUP_SUCCESS = "backup_success"
    PROGRESS_PARTTERN = "progress_parttern"


class WhitePath:
    MOUNT_PATH = "/mnt/databackup/"


class BackupStatus:
    COMPLETED = 1
    RUNNING = 2
    FAILED = 3
